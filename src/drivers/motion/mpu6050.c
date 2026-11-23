#include "edev/drivers/motion/mpu6050.h"

#define REG_SELF_TEST_X  0x0D
#define REG_SELF_TEST_Y  0x0E
#define REG_SELF_TEST_Z  0x0F
#define REG_SELF_TEST_A  0x10
#define REG_SMPLRT_DIV   0x19
#define REG_CONFIG       0x1A
#define REG_GYRO_CONFIG  0x1B
#define REG_ACCEL_CONFIG 0x1C
#define REG_FIFO_EN      0x23
#define REG_I2C_MST_CTRL 0x24
#define REG_INT_ENABLE   0x38
#define REG_INT_STATUS   0x3A
#define REG_ACCEL_OUT    0x3B
#define REG_TEMP_OUT     0x41
#define REG_GYRO_OUT     0x43
#define REG_USER_CTRL    0x6A
#define REG_PWR_MGMT_1   0x6B
#define REG_PWR_MGMT_2   0x6C
#define REG_BANK_SEL     0x6D
#define REG_MEM_START    0x6E
#define REG_MEM_R_W      0x6F
#define REG_FIFO_COUNT   0x72
#define REG_FIFO_R_W     0x74
#define REG_WHO_AM_I     0x75

static int read_bytes(mpu6050_t *d, uint8_t reg, uint8_t *buf, size_t len) {
    if (!d || !buf || !len) return EDEV_ERR_INVAL;
    return edev_i2c_dev_read_regs(&d->dev, reg, buf, len);
}

static int write_byte(mpu6050_t *d, uint8_t reg, uint8_t value) {
    if (!d) return EDEV_ERR_INVAL;
    return edev_i2c_dev_write_reg(&d->dev, reg, value);
}

static int update_bits(mpu6050_t *d, uint8_t reg, uint8_t mask, uint8_t value) {
    uint8_t current;
    int r = read_bytes(d, reg, &current, 1);
    if (r != EDEV_OK) return r;
    return write_byte(d, reg, (uint8_t)((current & (uint8_t)~mask) | (value & mask)));
}

static int16_t be16(const uint8_t *p) {
    return (int16_t)(((uint16_t)p[0] << 8) | p[1]);
}

int mpu6050_init(mpu6050_t *d, edev_i2c_bus_t *bus, uint8_t addr) {
    if (!d || !bus || (addr != MPU6050_ADDR_AD0_LOW && addr != MPU6050_ADDR_AD0_HIGH))
        return EDEV_ERR_INVAL;
    edev_i2c_dev_init(&d->dev, bus, addr);
    d->accel_range = MPU6050_ACCEL_2G;
    d->gyro_range = MPU6050_GYRO_250_DPS;
    uint8_t id;
    int r = read_bytes(d, REG_WHO_AM_I, &id, 1);
    if (r != EDEV_OK) return r;
    if (id != MPU6050_WHO_AM_I && id != MPU6500_WHO_AM_I) return EDEV_ERR_IO;
    d->who_am_i = id;
    r = write_byte(d, REG_PWR_MGMT_1, 0x01);
    if (r != EDEV_OK) return r;
    r = write_byte(d, REG_PWR_MGMT_2, 0x00);
    if (r != EDEV_OK) return r;
    r = write_byte(d, REG_CONFIG, 0x00);
    if (r != EDEV_OK) return r;
    r = write_byte(d, REG_SMPLRT_DIV, 0x00);
    if (r != EDEV_OK) return r;
    r = write_byte(d, REG_ACCEL_CONFIG, 0x00);
    if (r != EDEV_OK) return r;
    return write_byte(d, REG_GYRO_CONFIG, 0x00);
}

int mpu6050_reset(mpu6050_t *d) {
    int r = write_byte(d, REG_PWR_MGMT_1, 0x80);
    if (r == EDEV_OK) {
        d->accel_range = MPU6050_ACCEL_2G;
        d->gyro_range = MPU6050_GYRO_250_DPS;
    }
    return r;
}

int mpu6050_read_register(mpu6050_t *d, uint8_t reg, uint8_t *value) {
    return read_bytes(d, reg, value, 1);
}

int mpu6050_write_register(mpu6050_t *d, uint8_t reg, uint8_t value) {
    return write_byte(d, reg, value);
}

int mpu6050_set_sleep(mpu6050_t *d, bool sleep) {
    return update_bits(d, REG_PWR_MGMT_1, 0x40, sleep ? 0x40 : 0x00);
}

int mpu6050_set_clock_source(mpu6050_t *d, uint8_t source) {
    if (source > 7) return EDEV_ERR_INVAL;
    return update_bits(d, REG_PWR_MGMT_1, 0x07, source);
}

int mpu6050_set_sample_rate_divider(mpu6050_t *d, uint8_t divider) {
    return write_byte(d, REG_SMPLRT_DIV, divider);
}

int mpu6050_set_dlpf(mpu6050_t *d, mpu6050_dlpf_t dlpf) {
    if ((unsigned)dlpf > MPU6050_DLPF_5_HZ) return EDEV_ERR_INVAL;
    return update_bits(d, REG_CONFIG, 0x07, (uint8_t)dlpf);
}

int mpu6050_set_accel_range(mpu6050_t *d, mpu6050_accel_range_t range) {
    if (!d || (unsigned)range > MPU6050_ACCEL_16G) return EDEV_ERR_INVAL;
    int r = update_bits(d, REG_ACCEL_CONFIG, 0x18, (uint8_t)range << 3);
    if (r == EDEV_OK) d->accel_range = range;
    return r;
}

int mpu6050_set_gyro_range(mpu6050_t *d, mpu6050_gyro_range_t range) {
    if (!d || (unsigned)range > MPU6050_GYRO_2000_DPS) return EDEV_ERR_INVAL;
    int r = update_bits(d, REG_GYRO_CONFIG, 0x18, (uint8_t)range << 3);
    if (r == EDEV_OK) d->gyro_range = range;
    return r;
}

int mpu6050_read_raw(mpu6050_t *d, mpu6050_raw_sample_t *s) {
    if (!s) return EDEV_ERR_INVAL;
    uint8_t b[14];
    int r = read_bytes(d, REG_ACCEL_OUT, b, sizeof(b));
    if (r != EDEV_OK) return r;
    s->accel_x = be16(&b[0]); s->accel_y = be16(&b[2]); s->accel_z = be16(&b[4]);
    s->temperature = be16(&b[6]);
    s->gyro_x = be16(&b[8]); s->gyro_y = be16(&b[10]); s->gyro_z = be16(&b[12]);
    return EDEV_OK;
}

int mpu6050_read(mpu6050_t *d, mpu6050_sample_t *s) {
    if (!d || !s) return EDEV_ERR_INVAL;
    mpu6050_raw_sample_t raw;
    int r = mpu6050_read_raw(d, &raw);
    if (r != EDEV_OK) return r;
    static const float accel_lsb_per_g[] = {16384.0f, 8192.0f, 4096.0f, 2048.0f};
    static const float gyro_lsb_per_dps[] = {131.0f, 65.5f, 32.8f, 16.4f};
    if ((unsigned)d->accel_range > 3 || (unsigned)d->gyro_range > 3) return EDEV_ERR_INVAL;
    float as = accel_lsb_per_g[d->accel_range], gs = gyro_lsb_per_dps[d->gyro_range];
    s->accel_x_g = raw.accel_x / as; s->accel_y_g = raw.accel_y / as; s->accel_z_g = raw.accel_z / as;
    s->temperature_c = raw.temperature / 340.0f + 36.53f;
    s->gyro_x_dps = raw.gyro_x / gs; s->gyro_y_dps = raw.gyro_y / gs; s->gyro_z_dps = raw.gyro_z / gs;
    return EDEV_OK;
}

int mpu6050_read_self_test_codes(mpu6050_t *d, uint8_t accel_codes[3], uint8_t gyro_codes[3]) {
    if (!d || !accel_codes || !gyro_codes) return EDEV_ERR_INVAL;
    uint8_t a, x, y, z;
    int r = read_bytes(d, REG_SELF_TEST_A, &a, 1); if (r != EDEV_OK) return r;
    r = read_bytes(d, REG_SELF_TEST_X, &x, 1); if (r != EDEV_OK) return r;
    r = read_bytes(d, REG_SELF_TEST_Y, &y, 1); if (r != EDEV_OK) return r;
    r = read_bytes(d, REG_SELF_TEST_Z, &z, 1); if (r != EDEV_OK) return r;
    accel_codes[0] = (uint8_t)((((x >> 5) & 7) << 2) | ((a >> 4) & 3));
    accel_codes[1] = (uint8_t)((((y >> 5) & 7) << 2) | ((a >> 2) & 3));
    accel_codes[2] = (uint8_t)((((z >> 5) & 7) << 2) | (a & 3));
    gyro_codes[0] = x & 0x1F; gyro_codes[1] = y & 0x1F; gyro_codes[2] = z & 0x1F;
    return EDEV_OK;
}

static int write_offset(mpu6050_t *d, uint8_t reg, int16_t offset) {
    uint8_t bytes[2] = {(uint8_t)((uint16_t)offset >> 8), (uint8_t)offset};
    if (!d) return EDEV_ERR_INVAL;
    return edev_i2c_dev_write(&d->dev, (uint8_t[]){reg, bytes[0], bytes[1]}, 3);
}

int mpu6050_write_accel_offset(mpu6050_t *d, uint8_t axis, int16_t offset) {
    static const uint8_t regs[] = {0x06, 0x08, 0x0A};
    if (axis > 2) return EDEV_ERR_INVAL;
    return write_offset(d, regs[axis], offset);
}

int mpu6050_write_gyro_offset(mpu6050_t *d, uint8_t axis, int16_t offset) {
    static const uint8_t regs[] = {0x13, 0x15, 0x17};
    if (axis > 2) return EDEV_ERR_INVAL;
    return write_offset(d, regs[axis], offset);
}

int mpu6050_set_interrupt_enable(mpu6050_t *d, uint8_t mask) {
    return write_byte(d, REG_INT_ENABLE, mask);
}

int mpu6050_read_interrupt_status(mpu6050_t *d, uint8_t *status) {
    return read_bytes(d, REG_INT_STATUS, status, 1);
}

int mpu6050_fifo_set_enabled(mpu6050_t *d, bool enabled) {
    return update_bits(d, REG_USER_CTRL, 0x40, enabled ? 0x40 : 0x00);
}

int mpu6050_set_fifo_sources(mpu6050_t *d, uint8_t mask) {
    return write_byte(d, REG_FIFO_EN, mask);
}

int mpu6050_fifo_reset(mpu6050_t *d) {
    return update_bits(d, REG_USER_CTRL, 0x04, 0x04);
}

int mpu6050_fifo_count(mpu6050_t *d, uint16_t *count) {
    uint8_t b[2];
    if (!count) return EDEV_ERR_INVAL;
    int r = read_bytes(d, REG_FIFO_COUNT, b, 2);
    if (r == EDEV_OK) *count = (uint16_t)(((uint16_t)b[0] << 8) | b[1]);
    return r;
}

int mpu6050_fifo_read(mpu6050_t *d, uint8_t *buf, size_t len) {
    if (!d || !buf || !len) return EDEV_ERR_INVAL;
    return read_bytes(d, REG_FIFO_R_W, buf, len);
}

static int memory_transfer(mpu6050_t *d, uint16_t address, uint8_t *buf, size_t len, bool write) {
    if (!d || (!buf && len) || (size_t)address + len > MPU6050_DMP_MEMORY_SIZE)
        return EDEV_ERR_INVAL;
    while (len) {
        uint8_t bank = (uint8_t)(address >> 8), offset = (uint8_t)address;
        size_t chunk = 256u - offset;
        if (chunk > len) chunk = len;
        int r = write_byte(d, REG_BANK_SEL, bank);
        if (r != EDEV_OK) return r;
        r = write_byte(d, REG_MEM_START, offset);
        if (r != EDEV_OK) return r;
        if (write) {
            while (chunk) {
                size_t n = chunk > 16 ? 16 : chunk;
                uint8_t packet[17]; packet[0] = REG_MEM_R_W;
                for (size_t i = 0; i < n; ++i) packet[i + 1] = buf[i];
                r = edev_i2c_dev_write(&d->dev, packet, n + 1);
                if (r != EDEV_OK) return r;
                address += (uint16_t)n; buf += n; len -= n; chunk -= n;
                if (chunk) {
                    r = write_byte(d, REG_BANK_SEL, (uint8_t)(address >> 8));
                    if (r != EDEV_OK) return r;
                    r = write_byte(d, REG_MEM_START, (uint8_t)address);
                    if (r != EDEV_OK) return r;
                }
            }
        } else {
            r = read_bytes(d, REG_MEM_R_W, buf, chunk);
            if (r != EDEV_OK) return r;
            address += (uint16_t)chunk; buf += chunk; len -= chunk;
        }
    }
    return EDEV_OK;
}

int mpu6050_dmp_write_memory(mpu6050_t *d, uint16_t address, const uint8_t *data, size_t len) {
    return memory_transfer(d, address, (uint8_t *)data, len, true);
}

int mpu6050_dmp_read_memory(mpu6050_t *d, uint16_t address, uint8_t *data, size_t len) {
    return memory_transfer(d, address, data, len, false);
}

int mpu6050_dmp_upload(mpu6050_t *d, const uint8_t *image, size_t len) {
    if (!d || !image || !len || len > MPU6050_DMP_MEMORY_SIZE) return EDEV_ERR_INVAL;
    int r = mpu6050_dmp_set_enabled(d, false);
    if (r != EDEV_OK) return r;
    r = mpu6050_dmp_write_memory(d, 0, image, len);
    if (r != EDEV_OK) return r;
    return mpu6050_fifo_reset(d);
}

int mpu6050_dmp_set_enabled(mpu6050_t *d, bool enabled) {
    return update_bits(d, REG_USER_CTRL, 0xC0, enabled ? 0xC0 : 0x00);
}

int mpu6050_aux_i2c_set_enabled(mpu6050_t *d, bool enabled) {
    return update_bits(d, REG_USER_CTRL, 0x20, enabled ? 0x20 : 0x00);
}

int mpu6050_aux_i2c_set_clock(mpu6050_t *d, uint8_t clock) {
    if (clock > 15) return EDEV_ERR_INVAL;
    return update_bits(d, REG_I2C_MST_CTRL, 0x0F, clock);
}
