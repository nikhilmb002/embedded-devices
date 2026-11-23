#ifndef EDEV_MPU6050_H
#define EDEV_MPU6050_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "edev/bus/i2c_dev.h"

#define MPU6050_ADDR_AD0_LOW  0x68
#define MPU6050_ADDR_AD0_HIGH 0x69
#define MPU6050_WHO_AM_I      0x68
#define MPU6050_DMP_MEMORY_SIZE 2048u

typedef enum {
    MPU6050_ACCEL_2G = 0,
    MPU6050_ACCEL_4G = 1,
    MPU6050_ACCEL_8G = 2,
    MPU6050_ACCEL_16G = 3
} mpu6050_accel_range_t;

typedef enum {
    MPU6050_GYRO_250_DPS = 0,
    MPU6050_GYRO_500_DPS = 1,
    MPU6050_GYRO_1000_DPS = 2,
    MPU6050_GYRO_2000_DPS = 3
} mpu6050_gyro_range_t;

typedef enum {
    MPU6050_DLPF_260_HZ = 0,
    MPU6050_DLPF_184_HZ = 1,
    MPU6050_DLPF_94_HZ = 2,
    MPU6050_DLPF_44_HZ = 3,
    MPU6050_DLPF_21_HZ = 4,
    MPU6050_DLPF_10_HZ = 5,
    MPU6050_DLPF_5_HZ = 6
} mpu6050_dlpf_t;

typedef struct {
    int16_t accel_x, accel_y, accel_z;
    int16_t temperature;
    int16_t gyro_x, gyro_y, gyro_z;
} mpu6050_raw_sample_t;

typedef struct {
    float accel_x_g, accel_y_g, accel_z_g;
    float temperature_c;
    float gyro_x_dps, gyro_y_dps, gyro_z_dps;
} mpu6050_sample_t;

typedef struct {
    edev_i2c_dev_t dev;
    mpu6050_accel_range_t accel_range;
    mpu6050_gyro_range_t gyro_range;
} mpu6050_t;

/* Initialization verifies WHO_AM_I, wakes the device, and selects ±2 g/±250 dps. */
int mpu6050_init(mpu6050_t *d, edev_i2c_bus_t *bus, uint8_t addr);
int mpu6050_reset(mpu6050_t *d);
int mpu6050_read_register(mpu6050_t *d, uint8_t reg, uint8_t *value);
int mpu6050_write_register(mpu6050_t *d, uint8_t reg, uint8_t value);

int mpu6050_set_sleep(mpu6050_t *d, bool sleep);
int mpu6050_set_clock_source(mpu6050_t *d, uint8_t source); /* 0..7 */
int mpu6050_set_sample_rate_divider(mpu6050_t *d, uint8_t divider);
int mpu6050_set_dlpf(mpu6050_t *d, mpu6050_dlpf_t dlpf);
int mpu6050_set_accel_range(mpu6050_t *d, mpu6050_accel_range_t range);
int mpu6050_set_gyro_range(mpu6050_t *d, mpu6050_gyro_range_t range);

int mpu6050_read_raw(mpu6050_t *d, mpu6050_raw_sample_t *sample);
int mpu6050_read(mpu6050_t *d, mpu6050_sample_t *sample);
int mpu6050_read_self_test_codes(mpu6050_t *d, uint8_t accel_codes[3], uint8_t gyro_codes[3]);
int mpu6050_write_accel_offset(mpu6050_t *d, uint8_t axis, int16_t offset);
int mpu6050_write_gyro_offset(mpu6050_t *d, uint8_t axis, int16_t offset);

int mpu6050_set_interrupt_enable(mpu6050_t *d, uint8_t mask);
int mpu6050_read_interrupt_status(mpu6050_t *d, uint8_t *status);
int mpu6050_fifo_set_enabled(mpu6050_t *d, bool enabled);
int mpu6050_set_fifo_sources(mpu6050_t *d, uint8_t mask);
int mpu6050_fifo_reset(mpu6050_t *d);
int mpu6050_fifo_count(mpu6050_t *d, uint16_t *count);
int mpu6050_fifo_read(mpu6050_t *d, uint8_t *buf, size_t len);

/* DMP memory upload/control. Firmware images and packet formats are caller-owned. */
int mpu6050_dmp_write_memory(mpu6050_t *d, uint16_t address,
                             const uint8_t *data, size_t len);
int mpu6050_dmp_read_memory(mpu6050_t *d, uint16_t address,
                            uint8_t *data, size_t len);
int mpu6050_dmp_upload(mpu6050_t *d, const uint8_t *image, size_t len);
int mpu6050_dmp_set_enabled(mpu6050_t *d, bool enabled);

/* Enable/configure the MPU auxiliary I²C master. Individual slave registers
 * remain accessible through the generic register read/write functions. */
int mpu6050_aux_i2c_set_enabled(mpu6050_t *d, bool enabled);
int mpu6050_aux_i2c_set_clock(mpu6050_t *d, uint8_t clock);

#endif
