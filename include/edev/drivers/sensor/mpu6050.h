#ifndef EDEV_MPU6050_H
#define EDEV_MPU6050_H

#include <stdint.h>
#include <stdbool.h>
#include "edev/bus/i2c_dev.h"
#include "edev/drivers/sensor/mpu6050_regs.h"

/* ---- Types (must come before the struct that uses them) ---- */
typedef enum {
    MPU6050_CHIP_MPU6050 = 0,   /* WHO_AM_I 0x68 */
    MPU6050_CHIP_MPU6500 = 1,   /* WHO_AM_I 0x70 (common "MPU6050" clone) */
    MPU6050_CHIP_OTHER   = 2
} mpu6050_chip_t;

typedef enum {
    MPU6050_ACCEL_2G  = 0,
    MPU6050_ACCEL_4G  = 1,
    MPU6050_ACCEL_8G  = 2,
    MPU6050_ACCEL_16G = 3
} mpu6050_accel_range_t;

typedef enum {
    MPU6050_GYRO_250DPS  = 0,
    MPU6050_GYRO_500DPS  = 1,
    MPU6050_GYRO_1000DPS = 2,
    MPU6050_GYRO_2000DPS = 3
} mpu6050_gyro_range_t;

/* Digital low-pass filter (CONFIG bits 2:0). Higher = more smoothing, more delay. */
typedef enum {
    MPU6050_DLPF_260HZ = 0,   /* effectively off */
    MPU6050_DLPF_184HZ = 1,
    MPU6050_DLPF_94HZ  = 2,
    MPU6050_DLPF_44HZ  = 3,
    MPU6050_DLPF_21HZ  = 4,
    MPU6050_DLPF_10HZ  = 5,
    MPU6050_DLPF_5HZ   = 6
} mpu6050_dlpf_t;

/* The caller owns this struct. No hidden globals. */
typedef struct {
    edev_i2c_dev_t        dev;
    mpu6050_chip_t        chip;
    uint8_t               who_am_i;
    mpu6050_accel_range_t accel_range;
    mpu6050_gyro_range_t  gyro_range;
    uint8_t               dlpf;
} mpu6050_t;

/* ---- Core ---- */
int mpu6050_init(mpu6050_t *m, edev_i2c_bus_t *bus, uint8_t addr);
int mpu6050_who_am_i(mpu6050_t *m, uint8_t *id);
int mpu6050_reset(mpu6050_t *m);
int mpu6050_sleep(mpu6050_t *m, bool sleep);
int mpu6050_update_bits(mpu6050_t *m, uint8_t reg, uint8_t mask, uint8_t value);

/* ---- Configuration ---- */
int mpu6050_set_accel_range(mpu6050_t *m, mpu6050_accel_range_t range);
int mpu6050_set_gyro_range (mpu6050_t *m, mpu6050_gyro_range_t range);
int mpu6050_set_dlpf       (mpu6050_t *m, mpu6050_dlpf_t dlpf);

/* Sample rate in Hz. Base rate is 1 kHz with the DLPF on, 8 kHz with it at 260 Hz. */
int mpu6050_set_sample_rate(mpu6050_t *m, uint16_t hz);

#endif /* EDEV_MPU6050_H */