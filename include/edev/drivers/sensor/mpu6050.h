#ifndef EDEV_MPU6050_H
#define EDEV_MPU6050_H

#include <stdint.h>
#include <stdbool.h>
#include "edev/bus/i2c_dev.h"
#include "edev/drivers/sensor/mpu6050_regs.h"

typedef enum {
    MPU6050_CHIP_MPU6050 = 0,   /* WHO_AM_I 0x68 */
    MPU6050_CHIP_MPU6500 = 1,   /* WHO_AM_I 0x70 (common "MPU6050" clone) */
    MPU6050_CHIP_OTHER   = 2    /* unknown but responds: use at your own risk */
} mpu6050_chip_t;

typedef struct {
    edev_i2c_dev_t dev;
    mpu6050_chip_t chip;
    uint8_t        who_am_i;     /* the raw ID the chip returned */
} mpu6050_t;

/*
 * Initialise the sensor:
 *   1. check the chip answers and WHO_AM_I == 0x68
 *   2. reset it, then wake it from sleep
 *   3. select the gyro-X PLL as clock (more stable than the internal oscillator)
 * addr is MPU6050_ADDR_AD0_LOW (0x68) or MPU6050_ADDR_AD0_HIGH (0x69).
 * Returns EDEV_OK or a negative EDEV_ERR_* code.
 * The caller must provide a delay of at least 100 ms between power-up and this call.
 */
int mpu6050_init(mpu6050_t *m, edev_i2c_bus_t *bus, uint8_t addr);

/* Read the WHO_AM_I register (should be 0x68). */
int mpu6050_who_am_i(mpu6050_t *m, uint8_t *id);

/* Software reset: all registers return to defaults and the chip goes to sleep. */
int mpu6050_reset(mpu6050_t *m);

/* Sleep mode on/off. After reset or power-up the chip is asleep. */
int mpu6050_sleep(mpu6050_t *m, bool sleep);

/* Low-level helpers: set/clear bits in a register (read-modify-write). */
int mpu6050_update_bits(mpu6050_t *m, uint8_t reg, uint8_t mask, uint8_t value);

#endif /* EDEV_MPU6050_H */