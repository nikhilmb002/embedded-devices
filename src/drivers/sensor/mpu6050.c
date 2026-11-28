#include "edev/drivers/sensor/mpu6050.h"
#include "edev/edev_delay.h"

int mpu6050_update_bits(mpu6050_t *m, uint8_t reg, uint8_t mask, uint8_t value) {
    
    if (!m) return EDEV_ERR_INVAL;
    uint8_t v;
    int r = edev_i2c_dev_read_regs(&m->dev, reg, &v, 1);
    if (r != EDEV_OK) return r;
    v = (uint8_t)((v & ~mask) | (value & mask));
    return edev_i2c_dev_write_reg(&m->dev, reg, v);
}

int mpu6050_who_am_i(mpu6050_t *m, uint8_t *id) {

    if (!m || !id) return EDEV_ERR_INVAL;
    return edev_i2c_dev_read_regs(&m->dev, MPU6050_REG_WHO_AM_I, id, 1);
}

int mpu6050_reset(mpu6050_t *m) {

    if (!m) return EDEV_ERR_INVAL;
    int r = edev_i2c_dev_write_reg(&m->dev, MPU6050_REG_PWR_MGMT_1, MPU6050_PWR1_DEVICE_RESET);
    if (r != EDEV_OK) return r;
    edev_delay_ms(100);                       /* datasheet: wait for reset to finish */
    return EDEV_OK;
}

int mpu6050_sleep(mpu6050_t *m, bool sleep) {

    return mpu6050_update_bits(m, MPU6050_REG_PWR_MGMT_1, MPU6050_PWR1_SLEEP,
                               sleep ? MPU6050_PWR1_SLEEP : 0);
}

int mpu6050_init(mpu6050_t *m, edev_i2c_bus_t *bus, uint8_t addr) {

    if (!m || !bus) return EDEV_ERR_INVAL;
    
    edev_i2c_dev_init(&m->dev, bus, addr);

    int r = edev_i2c_dev_probe(&m->dev);           /* does anything ACK this address? */
    
    if (r != EDEV_OK) return r;

    uint8_t id = 0;
    r = mpu6050_who_am_i(m, &id);
    
    if (r != EDEV_OK) return r;
    
    m->who_am_i = id;
    switch (id) {
    case 0x68: m->chip = MPU6050_CHIP_MPU6050; break;
    case 0x70: m->chip = MPU6050_CHIP_MPU6500; break;
    default:   m->chip = MPU6050_CHIP_OTHER;   break;   /* still continue */
    }

    r = mpu6050_reset(m);
    
    if (r != EDEV_OK) return r;

    /* Wake up and select the gyro-X PLL clock in a single write:
     * SLEEP = 0, CLKSEL = 1. */
    r = edev_i2c_dev_write_reg(&m->dev, MPU6050_REG_PWR_MGMT_1, 0x01);
    if (r != EDEV_OK) return r;

    /* known defaults after reset */
    m->accel_range = MPU6050_ACCEL_2G;
    m->gyro_range  = MPU6050_GYRO_250DPS;
    m->dlpf        = MPU6050_DLPF_260HZ;
    m->bias = (mpu6050_raw_t){0};

    return EDEV_OK;
}

int mpu6050_set_accel_range(mpu6050_t *m, mpu6050_accel_range_t range) {

    if (!m || range > MPU6050_ACCEL_16G) return EDEV_ERR_INVAL;
    int r = mpu6050_update_bits(m, MPU6050_REG_ACCEL_CONFIG, MPU6050_FS_SEL_MASK,
                                (uint8_t)(range << MPU6050_FS_SEL_SHIFT));
    if (r == EDEV_OK) m->accel_range = range;
    return r;
}

int mpu6050_set_gyro_range(mpu6050_t *m, mpu6050_gyro_range_t range) {

    if (!m || range > MPU6050_GYRO_2000DPS) return EDEV_ERR_INVAL;
    int r = mpu6050_update_bits(m, MPU6050_REG_GYRO_CONFIG, MPU6050_FS_SEL_MASK,
                                (uint8_t)(range << MPU6050_FS_SEL_SHIFT));
    if (r == EDEV_OK) m->gyro_range = range;
    return r;
}

int mpu6050_set_dlpf(mpu6050_t *m, mpu6050_dlpf_t dlpf) {

    if (!m || dlpf > MPU6050_DLPF_5HZ) return EDEV_ERR_INVAL;
    int r = mpu6050_update_bits(m, MPU6050_REG_CONFIG, MPU6050_DLPF_CFG_MASK, (uint8_t)dlpf);
    if (r == EDEV_OK) m->dlpf = (uint8_t)dlpf;
    return r;
}

int mpu6050_set_sample_rate(mpu6050_t *m, uint16_t hz) {

    if (!m || hz == 0) return EDEV_ERR_INVAL;
    uint16_t base = (m->dlpf == MPU6050_DLPF_260HZ) ? 8000 : 1000;
    uint16_t div  = (uint16_t)(base / hz);          /* rate = base / (1 + SMPLRT_DIV) */
    if (div < 1 || div > 256) return EDEV_ERR_INVAL;
    return edev_i2c_dev_write_reg(&m->dev, MPU6050_REG_SMPLRT_DIV, (uint8_t)(div - 1));
}

static int16_t be16(const uint8_t *p) {

    return (int16_t)(((uint16_t)p[0] << 8) | p[1]);   /* chip sends high byte first */
}

int mpu6050_read_raw(mpu6050_t *m, mpu6050_raw_t *raw) {

    if (!m || !raw) return EDEV_ERR_INVAL;
    
    uint8_t b[14];
    int r = edev_i2c_dev_read_regs(&m->dev, MPU6050_REG_ACCEL_XOUT_H, b, sizeof(b));
    
    if (r != EDEV_OK) return r;

    raw->ax   = (int16_t)(be16(&b[0])  - m->bias.ax);
    raw->ay   = (int16_t)(be16(&b[2])  - m->bias.ay);
    raw->az   = (int16_t)(be16(&b[4])  - m->bias.az);
    raw->temp = be16(&b[6]);
    raw->gx   = (int16_t)(be16(&b[8])  - m->bias.gx);
    raw->gy   = (int16_t)(be16(&b[10]) - m->bias.gy);
    raw->gz   = (int16_t)(be16(&b[12]) - m->bias.gz);

    return EDEV_OK;
}

void mpu6050_convert(const mpu6050_t *m, const mpu6050_raw_t *raw, mpu6050_data_t *out) {

    /* LSB per g and per deg/s for range settings 0..3 */
    static const float accel_lsb[4] = { 16384.0f, 8192.0f, 4096.0f, 2048.0f };
    static const float gyro_lsb[4]  = { 131.0f, 65.5f, 32.8f, 16.4f };

    float a = accel_lsb[m->accel_range];
    float g = gyro_lsb[m->gyro_range];

    out->ax = raw->ax / a;  out->ay = raw->ay / a;  out->az = raw->az / a;
    out->gx = raw->gx / g;  out->gy = raw->gy / g;  out->gz = raw->gz / g;

    if (m->chip == MPU6050_CHIP_MPU6500)
        out->temp_c = raw->temp / 333.87f + 21.0f;     /* MPU6500 formula */
    
    else
        out->temp_c = raw->temp / 340.0f + 36.53f;     /* MPU6050 formula */
}

int mpu6050_read(mpu6050_t *m, mpu6050_data_t *out) {

    if (!m || !out) return EDEV_ERR_INVAL;
    
    mpu6050_raw_t raw;
    int r = mpu6050_read_raw(m, &raw);
    
    if (r != EDEV_OK) return r;
    
    mpu6050_convert(m, &raw, out);
    return EDEV_OK;
}

int mpu6050_calibrate(mpu6050_t *m, uint16_t samples, bool level) {

    if (!m || samples == 0) return EDEV_ERR_INVAL;

    mpu6050_clear_bias(m);
    int32_t sx = 0, sy = 0, sz = 0, gx = 0, gy = 0, gz = 0;

    for (uint16_t i = 0; i < samples; i++) {
        mpu6050_raw_t r;
        int e = mpu6050_read_raw(m, &r);
        if (e != EDEV_OK) return e;
        sx += r.ax; sy += r.ay; sz += r.az;
        gx += r.gx; gy += r.gy; gz += r.gz;
        edev_delay_ms(5);
    }

    m->bias.ax = (int16_t)(sx / samples);
    m->bias.ay = (int16_t)(sy / samples);
    m->bias.az = (int16_t)(sz / samples);
    m->bias.gx = (int16_t)(gx / samples);
    m->bias.gy = (int16_t)(gy / samples);
    m->bias.gz = (int16_t)(gz / samples);

    if (level) {
        static const int16_t one_g[4] = { 16384, 8192, 4096, 2048 };
        m->bias.az = (int16_t)(m->bias.az - one_g[m->accel_range]);
    } else {
        m->bias.ax = m->bias.ay = m->bias.az = 0;   /* accel bias only valid when level */
    }
    return EDEV_OK;
}

void mpu6050_clear_bias(mpu6050_t *m) {

    if (m) m->bias = (mpu6050_raw_t){0};
}

int mpu6050_int_config(mpu6050_t *m, bool active_low, bool open_drain, bool latch) {

    uint8_t v = 0;
    
    if (active_low) v |= MPU6050_INT_LEVEL_LOW;
    if (open_drain) v |= MPU6050_INT_OPEN_DRAIN;
    if (latch)      v |= MPU6050_INT_LATCH_EN;
    
    v |= MPU6050_INT_RD_CLEAR;                       /* any read clears the status */
    uint8_t mask = MPU6050_INT_LEVEL_LOW | MPU6050_INT_OPEN_DRAIN |
                   MPU6050_INT_LATCH_EN | MPU6050_INT_RD_CLEAR;
    return mpu6050_update_bits(m, MPU6050_REG_INT_PIN_CFG, mask, v);
}

int mpu6050_int_enable(mpu6050_t *m, uint8_t mask) {

    if (!m) return EDEV_ERR_INVAL;
    return edev_i2c_dev_write_reg(&m->dev, MPU6050_REG_INT_ENABLE, mask);
}

int mpu6050_int_status(mpu6050_t *m, uint8_t *status) {

    if (!m || !status) return EDEV_ERR_INVAL;
    return edev_i2c_dev_read_regs(&m->dev, MPU6050_REG_INT_STATUS, status, 1);
}

int mpu6050_motion_config(mpu6050_t *m, uint8_t threshold, uint8_t duration_ms) {

    if (!m) return EDEV_ERR_INVAL;
    
    int r = edev_i2c_dev_write_reg(&m->dev, MPU6050_REG_MOT_THR, threshold);
    
    if (r != EDEV_OK) return r;
    if (m->chip == MPU6050_CHIP_MPU6500)
        /* ACCEL_INTEL_CTRL: enable intelligence (bit 7) + compare with previous sample (bit 6) */
        return edev_i2c_dev_write_reg(&m->dev, MPU6050_REG_MOT_DETECT_CTRL, 0xC0);
    return edev_i2c_dev_write_reg(&m->dev, MPU6050_REG_MOT_DUR, duration_ms);
}

int mpu6050_freefall_config(mpu6050_t *m, uint8_t threshold, uint8_t duration_ms) {

    if (!m) return EDEV_ERR_INVAL;
    if (m->chip != MPU6050_CHIP_MPU6050) return EDEV_ERR_NOSUPPORT;
    
    int r = edev_i2c_dev_write_reg(&m->dev, MPU6050_REG_FF_THR, threshold);
    
    if (r != EDEV_OK) return r;
    return edev_i2c_dev_write_reg(&m->dev, MPU6050_REG_FF_DUR, duration_ms);
}

int mpu6050_zeromotion_config(mpu6050_t *m, uint8_t threshold, uint8_t duration_ms) {

    if (!m) return EDEV_ERR_INVAL;
    if (m->chip != MPU6050_CHIP_MPU6050) return EDEV_ERR_NOSUPPORT;
    
    int r = edev_i2c_dev_write_reg(&m->dev, MPU6050_REG_ZRMOT_THR, threshold);
    
    if (r != EDEV_OK) return r;
    return edev_i2c_dev_write_reg(&m->dev, MPU6050_REG_ZRMOT_DUR, duration_ms);
}