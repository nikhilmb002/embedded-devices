#ifndef EDEV_MPU6050_H
#define EDEV_MPU6050_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "edev/bus/i2c_dev.h"

#define MPU6050_ADDR_AD0_LOW  0x68
#define MPU6050_ADDR_AD0_HIGH 0x69
#define MPU6050_WHO_AM_I      0x68
#define MPU6500_WHO_AM_I      0x70
#define MPU6050_DMP_MEMORY_SIZE 2048u

/* FIFO_EN bits, INT_ENABLE/INT_STATUS bits, and PWR_MGMT_2 standby bits. */
#define MPU6050_FIFO_TEMP       0x80
#define MPU6050_FIFO_GYRO_X     0x40
#define MPU6050_FIFO_GYRO_Y     0x20
#define MPU6050_FIFO_GYRO_Z     0x10
#define MPU6050_FIFO_ACCEL      0x08
#define MPU6050_FIFO_SLAVE_2    0x04
#define MPU6050_FIFO_SLAVE_1    0x02
#define MPU6050_FIFO_SLAVE_0    0x01
#define MPU6050_INT_FIFO_OFLOW  0x10
#define MPU6050_INT_I2C_MST     0x08
#define MPU6050_INT_ZERO_MOTION 0x20
#define MPU6050_INT_MOTION      0x40
#define MPU6050_INT_FREEFALL    0x80
#define MPU6050_INT_DATA_READY  0x01
#define MPU6050_AUX_STATUS_PASS_THROUGH 0x80
#define MPU6050_AUX_STATUS_SLV4_DONE    0x40
#define MPU6050_AUX_STATUS_LOST_ARB    0x20
#define MPU6050_AUX_STATUS_SLV4_NACK   0x10
#define MPU6050_AUX_STATUS_SLV3_NACK   0x08
#define MPU6050_AUX_STATUS_SLV2_NACK   0x04
#define MPU6050_AUX_STATUS_SLV1_NACK   0x02
#define MPU6050_AUX_STATUS_SLV0_NACK   0x01
#define MPU6050_STANDBY_XA      0x20
#define MPU6050_STANDBY_YA      0x10
#define MPU6050_STANDBY_ZA      0x08
#define MPU6050_STANDBY_XG      0x04
#define MPU6050_STANDBY_YG      0x02
#define MPU6050_STANDBY_ZG      0x01

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

typedef enum {
    MPU6050_ACCEL_HPF_RESET = 0,
    MPU6050_ACCEL_HPF_5_HZ = 1,
    MPU6050_ACCEL_HPF_2_5_HZ = 2,
    MPU6050_ACCEL_HPF_1_25_HZ = 3,
    MPU6050_ACCEL_HPF_0_63_HZ = 4,
    MPU6050_ACCEL_HPF_HOLD = 7
} mpu6050_accel_hpf_t;

typedef enum {
    MPU6050_WAKE_1_25_HZ = 0,
    MPU6050_WAKE_5_HZ = 1,
    MPU6050_WAKE_20_HZ = 2,
    MPU6050_WAKE_40_HZ = 3
} mpu6050_wake_rate_t;

typedef enum {
    MPU6050_SYNC_DISABLED = 0,
    MPU6050_SYNC_TEMP_OUT = 1,
    MPU6050_SYNC_GYRO_XOUT = 2,
    MPU6050_SYNC_GYRO_YOUT = 3,
    MPU6050_SYNC_GYRO_ZOUT = 4,
    MPU6050_SYNC_ACCEL_XOUT = 5,
    MPU6050_SYNC_ACCEL_YOUT = 6,
    MPU6050_SYNC_ACCEL_ZOUT = 7
} mpu6050_ext_sync_t;

typedef struct {
    bool active_low;
    bool open_drain;
    bool latched;
    bool clear_on_any_read;
    bool fsync_active_low;
    bool fsync_interrupt_enabled;
    bool bypass_enabled;
    bool clock_output_enabled;
} mpu6050_interrupt_pin_config_t;

typedef struct {
    uint8_t address;       /* 7-bit auxiliary sensor address */
    uint8_t reg;
    bool read;
    bool enabled;
    bool byte_swap;
    bool register_disabled;
    bool group_odd_to_even;
    uint8_t length;        /* 0..15 bytes (slave 0..3) */
} mpu6050_aux_slave_config_t;

typedef struct {
    bool multi_master;
    bool wait_for_external_sensor;
    bool slave3_fifo_enabled;
    bool stop_between_reads;
} mpu6050_aux_master_config_t;

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
    uint8_t who_am_i; /* identifies MPU6050 (0x68) or MPU6500 (0x70) */
} mpu6050_t;

/* Initialization accepts MPU6050/MPU6500, wakes the device, and selects
 * ±2 g/±250 dps. */
int mpu6050_init(mpu6050_t *d, edev_i2c_bus_t *bus, uint8_t addr);
/* Issue the device reset; caller must allow the sensor's reset/startup delay. */
int mpu6050_reset(mpu6050_t *d);
/* Reset sensor signal paths: bit 0=temp, bit 1=accel, bit 2=gyro. */
int mpu6050_reset_signal_path(mpu6050_t *d, uint8_t reset_mask);
int mpu6050_read_register(mpu6050_t *d, uint8_t reg, uint8_t *value);
int mpu6050_write_register(mpu6050_t *d, uint8_t reg, uint8_t value);

int mpu6050_set_sleep(mpu6050_t *d, bool sleep);
int mpu6050_set_cycle_mode(mpu6050_t *d, bool enabled);
int mpu6050_set_temperature_enabled(mpu6050_t *d, bool enabled);
int mpu6050_set_clock_source(mpu6050_t *d, uint8_t source); /* 0..7 */
int mpu6050_set_standby_axes(mpu6050_t *d, uint8_t standby_mask);
/* Wake rate is used while cycle mode is enabled. */
int mpu6050_set_wake_rate(mpu6050_t *d, mpu6050_wake_rate_t rate);
/* Sample rate = base rate / (1 + divider); base is selected by DLPF setting. */
int mpu6050_set_sample_rate_divider(mpu6050_t *d, uint8_t divider);
int mpu6050_set_dlpf(mpu6050_t *d, mpu6050_dlpf_t dlpf);
int mpu6050_set_external_sync(mpu6050_t *d, mpu6050_ext_sync_t sync);
int mpu6050_set_accel_high_pass_filter(mpu6050_t *d, mpu6050_accel_hpf_t filter);
int mpu6050_set_gyro_fchoice(mpu6050_t *d, uint8_t fchoice); /* 0..3 */
int mpu6050_set_accel_range(mpu6050_t *d, mpu6050_accel_range_t range);
int mpu6050_set_gyro_range(mpu6050_t *d, mpu6050_gyro_range_t range);
/* Axis masks use bit 0=X, bit 1=Y, bit 2=Z. */
int mpu6050_set_self_test_axes(mpu6050_t *d, uint8_t accel_axes, uint8_t gyro_axes);

int mpu6050_read_raw(mpu6050_t *d, mpu6050_raw_sample_t *sample);
int mpu6050_read(mpu6050_t *d, mpu6050_sample_t *sample);
int mpu6050_read_self_test_codes(mpu6050_t *d, uint8_t accel_codes[3], uint8_t gyro_codes[3]);
/* Offsets use chip register units; accel writes preserve the temperature-comp bit. */
int mpu6050_write_accel_offset(mpu6050_t *d, uint8_t axis, int16_t offset);
int mpu6050_write_gyro_offset(mpu6050_t *d, uint8_t axis, int16_t offset);
int mpu6050_read_accel_offset(mpu6050_t *d, uint8_t axis, int16_t *offset);
int mpu6050_read_gyro_offset(mpu6050_t *d, uint8_t axis, int16_t *offset);

/* Threshold is 0..510 mg in 2 mg steps; duration is in ms. */
int mpu6050_set_freefall_detection(mpu6050_t *d, uint16_t threshold_mg,
                                   uint8_t duration_ms);
int mpu6050_set_motion_detection(mpu6050_t *d, uint16_t threshold_mg,
                                 uint8_t duration_ms);
int mpu6050_set_zero_motion_detection(mpu6050_t *d, uint16_t threshold_mg,
                                      uint8_t duration_ms);
int mpu6050_set_motion_detection_control(mpu6050_t *d, uint8_t accel_on_delay,
                                         uint8_t freefall_count,
                                         uint8_t zero_motion_count);

int mpu6050_set_interrupt_enable(mpu6050_t *d, uint8_t mask);
int mpu6050_read_interrupt_status(mpu6050_t *d, uint8_t *status);
int mpu6050_read_aux_master_status(mpu6050_t *d, uint8_t *status);
int mpu6050_set_interrupt_pin_config(mpu6050_t *d,
                                     const mpu6050_interrupt_pin_config_t *config);
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
int mpu6050_aux_i2c_set_master_config(mpu6050_t *d,
                                      const mpu6050_aux_master_config_t *config);
int mpu6050_aux_i2c_set_bypass(mpu6050_t *d, bool enabled);
int mpu6050_aux_i2c_reset(mpu6050_t *d);
int mpu6050_aux_i2c_set_delay_control(mpu6050_t *d, uint8_t slave_mask,
                                      bool delay_external_sensor);
int mpu6050_aux_i2c_config_slave(mpu6050_t *d, uint8_t slave,
                                 const mpu6050_aux_slave_config_t *config);
int mpu6050_aux_i2c_config_slave4(mpu6050_t *d, uint8_t address, uint8_t reg,
                                  bool read, bool enabled, bool interrupt_enabled,
                                  bool register_disabled, uint8_t delay);
int mpu6050_aux_i2c_slave4_write_data(mpu6050_t *d, uint8_t value);
int mpu6050_aux_i2c_slave4_read_data(mpu6050_t *d, uint8_t *value);
int mpu6050_aux_i2c_read_external(mpu6050_t *d, uint8_t offset,
                                  uint8_t *data, size_t len);
int mpu6050_dmp_set_config(mpu6050_t *d, uint8_t config1, uint8_t config2);

#endif
