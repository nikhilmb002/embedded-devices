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
    return edev_i2c_dev_write_reg(&m->dev, MPU6050_REG_PWR_MGMT_1, 0x01);
}