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
