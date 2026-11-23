#include "edev/drivers/motion/mpu6050.h"
#include <assert.h>
#include <string.h>

typedef struct {
    uint8_t regs[256];
    uint8_t memory[MPU6050_DMP_MEMORY_SIZE];
    uint8_t reg;
    uint16_t mem_addr;
    int error;
} mock_t;

static int mock_write(edev_i2c_bus_t *bus, uint8_t addr, const uint8_t *buf,
                      size_t len, bool nostop) {
    (void)addr; (void)nostop;
    mock_t *m = bus->ctx;
    if (m->error) return m->error;
    if (len == 0) return EDEV_ERR_INVAL;
    m->reg = buf[0];
    if (m->reg == 0x6F && len > 1) {
        for (size_t i = 1; i < len; ++i) m->memory[m->mem_addr++ & 2047] = buf[i];
        return EDEV_OK;
    }
    for (size_t i = 1; i < len; ++i) {
        uint8_t reg = (uint8_t)(m->reg + i - 1);
        m->regs[reg] = buf[i];
        if (reg == 0x6D) m->mem_addr = (uint16_t)((buf[i] & 7) << 8) | (m->mem_addr & 0xFF);
        if (reg == 0x6E) m->mem_addr = (uint16_t)((m->mem_addr & 0x700) | buf[i]);
    }
    if (len > 1) m->reg = (uint8_t)(m->reg + len - 1);
    return EDEV_OK;
}

static int mock_read(edev_i2c_bus_t *bus, uint8_t addr, uint8_t *buf,
                     size_t len, bool nostop) {
    (void)addr; (void)nostop;
    mock_t *m = bus->ctx;
    if (m->error) return m->error;
    if (m->reg == 0x6F) {
        for (size_t i = 0; i < len; ++i) buf[i] = m->memory[m->mem_addr++ & 2047];
    } else {
        for (size_t i = 0; i < len; ++i) buf[i] = m->regs[(uint8_t)(m->reg + i)];
    }
    m->reg = (uint8_t)(m->reg + len);
    return EDEV_OK;
}

static const edev_i2c_ops_t ops = {mock_write, mock_read};

int main(void) {
    mock_t mock = {0};
    mock.regs[0x75] = 0x68;
    edev_i2c_bus_t bus = {&ops, &mock};
    mpu6050_t imu;
    assert(mpu6050_init(&imu, &bus, MPU6050_ADDR_AD0_LOW) == EDEV_OK);
    assert(mock.regs[0x6B] == 1);
    assert(imu.who_am_i == MPU6050_WHO_AM_I);

    mock.regs[0x75] = MPU6500_WHO_AM_I;
    assert(mpu6050_init(&imu, &bus, MPU6050_ADDR_AD0_LOW) == EDEV_OK);
    assert(imu.who_am_i == MPU6500_WHO_AM_I);
    mock.regs[0x75] = 0x71;
    assert(mpu6050_init(&imu, &bus, MPU6050_ADDR_AD0_LOW) == EDEV_ERR_IO);
    mock.regs[0x75] = MPU6050_WHO_AM_I;
    assert(mpu6050_init(&imu, &bus, MPU6050_ADDR_AD0_LOW) == EDEV_OK);

    const uint8_t raw[14] = {0x40,0x00, 0xC0,0x00, 0x20,0x00,
                             0x00,0x00, 0x00,0x83, 0xFF,0x7D, 0x00,0x00};
    memcpy(&mock.regs[0x3B], raw, sizeof(raw));
    mpu6050_sample_t sample;
    assert(mpu6050_read(&imu, &sample) == EDEV_OK);
    assert(sample.accel_x_g > 0.99f && sample.accel_x_g < 1.01f);
    assert(sample.accel_y_g < -0.99f && sample.accel_y_g > -1.01f);
    assert(sample.gyro_x_dps > 0.99f && sample.gyro_x_dps < 1.01f);
    assert(sample.gyro_y_dps < -0.99f && sample.gyro_y_dps > -1.01f);

    assert(mpu6050_set_accel_range(&imu, MPU6050_ACCEL_4G) == EDEV_OK);
    assert(imu.accel_range == MPU6050_ACCEL_4G);
    assert(mpu6050_set_dlpf(&imu, (mpu6050_dlpf_t)7) == EDEV_ERR_INVAL);

    assert(mpu6050_set_cycle_mode(&imu, true) == EDEV_OK);
    assert(mock.regs[0x6B] & 0x20);
    assert(mpu6050_set_temperature_enabled(&imu, false) == EDEV_OK);
    assert(mock.regs[0x6B] & 0x08);
    assert(mpu6050_set_wake_rate(&imu, MPU6050_WAKE_20_HZ) == EDEV_OK);
    assert((mock.regs[0x6C] & 0xC0) == 0x80);
    assert(mpu6050_set_standby_axes(&imu, MPU6050_STANDBY_XA | MPU6050_STANDBY_ZG) == EDEV_OK);
    assert((mock.regs[0x6C] & 0x3F) == 0x21);
    assert(mpu6050_set_external_sync(&imu, MPU6050_SYNC_GYRO_YOUT) == EDEV_OK);
    assert((mock.regs[0x1A] & 0x38) == 0x18);
    assert(mpu6050_set_accel_high_pass_filter(&imu, MPU6050_ACCEL_HPF_2_5_HZ) == EDEV_OK);
    assert((mock.regs[0x1C] & 0x07) == 2);
    assert(mpu6050_set_accel_high_pass_filter(&imu, (mpu6050_accel_hpf_t)5) == EDEV_ERR_INVAL);
    assert(mpu6050_set_gyro_fchoice(&imu, 2) == EDEV_OK);
    assert((mock.regs[0x1B] & 0x03) == 2);
    assert(mpu6050_set_self_test_axes(&imu, 5, 3) == EDEV_OK);
    assert((mock.regs[0x1C] & 0xE0) == 0xA0);
    assert((mock.regs[0x1B] & 0xE0) == 0xC0);
    assert(mpu6050_reset_signal_path(&imu, 0x06) == EDEV_OK);
    assert(mock.regs[0x68] == 0x06);

    assert(mpu6050_set_freefall_detection(&imu, 100, 20) == EDEV_OK);
    assert(mock.regs[0x1D] == 50 && mock.regs[0x1E] == 20);
    assert(mpu6050_set_motion_detection(&imu, 200, 30) == EDEV_OK);
    assert(mock.regs[0x1F] == 100 && mock.regs[0x20] == 30);
    assert(mpu6050_set_zero_motion_detection(&imu, 300, 40) == EDEV_OK);
    assert(mock.regs[0x21] == 150 && mock.regs[0x22] == 40);
    assert(mpu6050_set_motion_detection_control(&imu, 2, 1, 3) == EDEV_OK);
    assert((mock.regs[0x69] & 0x3F) == 0x27);

    mpu6050_interrupt_pin_config_t pin_cfg = {0};
    pin_cfg.active_low = true;
    pin_cfg.latched = true;
    assert(mpu6050_set_interrupt_pin_config(&imu, &pin_cfg) == EDEV_OK);
    assert(mock.regs[0x37] == 0xA0);
    assert(mpu6050_set_interrupt_enable(&imu, MPU6050_INT_DATA_READY |
                                        MPU6050_INT_MOTION) == EDEV_OK);
    assert(mock.regs[0x38] == 0x41);
    mock.regs[0x3A] = MPU6050_INT_MOTION;
    uint8_t status;
    assert(mpu6050_read_interrupt_status(&imu, &status) == EDEV_OK);
    assert(status == MPU6050_INT_MOTION);

    mock.regs[0x6A] = 0x40;
    assert(mpu6050_fifo_reset(&imu) == EDEV_OK);
    assert(mock.regs[0x6A] == 0x40);
    mock.regs[0x72] = 0xF1; mock.regs[0x73] = 0x23;
    uint16_t fifo_count;
    assert(mpu6050_fifo_count(&imu, &fifo_count) == EDEV_OK);
    assert(fifo_count == 0x123);
    assert(mpu6050_set_fifo_sources(&imu, MPU6050_FIFO_ACCEL | MPU6050_FIFO_TEMP) == EDEV_OK);
    assert(mock.regs[0x23] == 0x88);
    assert(mpu6050_fifo_set_enabled(&imu, true) == EDEV_OK);
    assert(mock.regs[0x6A] & 0x40);
    mock.regs[0x74] = 0x5A;
    assert(mpu6050_fifo_read(&imu, &status, 1) == EDEV_OK && status == 0x5A);

    mpu6050_aux_master_config_t master_cfg = {true, true, false, true};
    assert(mpu6050_aux_i2c_set_master_config(&imu, &master_cfg) == EDEV_OK);
    assert((mock.regs[0x24] & 0xF0) == 0xD0);
    assert(mpu6050_aux_i2c_set_clock(&imu, 9) == EDEV_OK);
    assert((mock.regs[0x24] & 0x0F) == 9);
    assert(mpu6050_aux_i2c_set_delay_control(&imu, 0x13, true) == EDEV_OK);
    assert(mock.regs[0x67] == 0x93);
    mock.regs[0x36] = MPU6050_AUX_STATUS_SLV0_NACK;
    assert(mpu6050_read_aux_master_status(&imu, &status) == EDEV_OK);
    assert(status == MPU6050_AUX_STATUS_SLV0_NACK);
    mpu6050_aux_slave_config_t slave_cfg = {0x0C, 0x0A, true, true,
                                             false, false, false, 6};
    assert(mpu6050_aux_i2c_config_slave(&imu, 2, &slave_cfg) == EDEV_OK);
    assert(mock.regs[0x2B] == 0x8C && mock.regs[0x2C] == 0x0A);
    assert(mock.regs[0x2D] == 0x86);
    assert(mpu6050_aux_i2c_config_slave4(&imu, 0x0C, 0x0B, true, true,
                                         true, true, 5) == EDEV_OK);
    assert(mock.regs[0x31] == 0x8C && mock.regs[0x34] == 0xE5);
    mock.regs[0x35] = 0x5A;
    uint8_t aux_data;
    assert(mpu6050_aux_i2c_slave4_read_data(&imu, &aux_data) == EDEV_OK);
    assert(aux_data == 0x5A);
    mock.regs[0x4A] = 0xA5;
    assert(mpu6050_aux_i2c_read_external(&imu, 1, &aux_data, 1) == EDEV_OK);
    assert(aux_data == 0xA5);
    assert(mpu6050_aux_i2c_set_bypass(&imu, true) == EDEV_OK);
    assert(mock.regs[0x37] & 0x02);
    assert(mpu6050_aux_i2c_set_enabled(&imu, true) == EDEV_OK);
    assert(mpu6050_aux_i2c_reset(&imu) == EDEV_OK);
    assert(mock.regs[0x6A] & 0x20);

    mock.regs[0x07] = 1;
    assert(mpu6050_write_accel_offset(&imu, 0, 0x1234) == EDEV_OK);
    assert(mock.regs[0x07] == 0x35);
    int16_t offset;
    assert(mpu6050_read_accel_offset(&imu, 0, &offset) == EDEV_OK);
    assert(offset == 0x1234);

    assert(mpu6050_dmp_set_config(&imu, 0x12, 0x34) == EDEV_OK);
    assert(mock.regs[0x70] == 0x12 && mock.regs[0x71] == 0x34);

    uint8_t image[20];
    for (unsigned i = 0; i < sizeof(image); ++i) image[i] = (uint8_t)i;
    assert(mpu6050_dmp_upload(&imu, image, sizeof(image)) == EDEV_OK);
    assert(memcmp(mock.memory, image, sizeof(image)) == 0);
    assert(mpu6050_dmp_write_memory(&imu, 2040, image, 16) == EDEV_ERR_INVAL);

    mock.error = EDEV_ERR_TIMEOUT;
    assert(mpu6050_read(&imu, &sample) == EDEV_ERR_TIMEOUT);
    mock.error = 0;
    assert(mpu6050_init(&imu, &bus, 0x67) == EDEV_ERR_INVAL);
    return 0;
}
