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

/* Raw sensor counts, exactly as the chip reports them. */
typedef struct {
    int16_t ax, ay, az;
    int16_t temp;
    int16_t gx, gy, gz;
} mpu6050_raw_t;

/* Converted values: acceleration in g, rotation in degrees/second, temperature in degC. */
typedef struct {
    float ax, ay, az;
    float gx, gy, gz;
    float temp_c;
} mpu6050_data_t;

/* The caller owns this struct. No hidden globals. */
typedef struct {
    edev_i2c_dev_t        dev;
    mpu6050_chip_t        chip;
    uint8_t               who_am_i;
    mpu6050_accel_range_t accel_range;
    mpu6050_gyro_range_t  gyro_range;
    uint8_t               dlpf;
    mpu6050_raw_t         bias;        /* subtracted in mpu6050_read_raw */
    uint8_t               fifo_sources;   /* what fifo_start selected */
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

/* One 14-byte burst: accel, temperature and gyro from the same sample. */
int mpu6050_read_raw (mpu6050_t *m, mpu6050_raw_t *raw);

/* Burst read plus conversion using the current ranges and chip type. */
int mpu6050_read     (mpu6050_t *m, mpu6050_data_t *out);

/* Conversion only (no I2C): useful for FIFO data and host tests. */
void mpu6050_convert (const mpu6050_t *m, const mpu6050_raw_t *raw, mpu6050_data_t *out);

/* Average 'samples' readings and store them as bias. Keep the board still.
 * If level is true, the board must lie flat (Z up): 1 g is removed from az
 * so the board then reads 0, 0, 1 g. Gyro bias is always removed. */
int mpu6050_calibrate(mpu6050_t *m, uint16_t samples, bool level);

/* Clear the bias. */
void mpu6050_clear_bias(mpu6050_t *m);

/* INT pin electrical configuration. */
int mpu6050_int_config(mpu6050_t *m, bool active_low, bool open_drain, bool latch);

/* Enable interrupt sources: OR of MPU6050_INT_DATA_RDY, _MOT, _FF, _ZMOT, _FIFO_OFLOW. */
int mpu6050_int_enable(mpu6050_t *m, uint8_t mask);

/* Read INT_STATUS (clears it). Bits are the same MPU6050_INT_* masks. */
int mpu6050_int_status(mpu6050_t *m, uint8_t *status);

/* Motion detection: threshold in units of 32 mg (MPU6050) or 4 mg (MPU6500 wake-on-motion),
 * duration in ms (MPU6050 only; ignored on MPU6500). */
int mpu6050_motion_config(mpu6050_t *m, uint8_t threshold, uint8_t duration_ms);

/* Free-fall and zero-motion: MPU6050 only (EDEV_ERR_NOSUPPORT on others). */
int mpu6050_freefall_config  (mpu6050_t *m, uint8_t threshold, uint8_t duration_ms);
int mpu6050_zeromotion_config(mpu6050_t *m, uint8_t threshold, uint8_t duration_ms);


/* Choose what goes into the FIFO: OR of MPU6050_FIFO_ACCEL, _GYRO_X/_Y/_Z, _TEMP.
 * Convenience: MPU6050_FIFO_ACCEL_GYRO = accel + all gyro axes (12 bytes/sample). */
#define MPU6050_FIFO_ACCEL_GYRO (MPU6050_FIFO_ACCEL | MPU6050_FIFO_GYRO_X | \
                                 MPU6050_FIFO_GYRO_Y | MPU6050_FIFO_GYRO_Z)

int mpu6050_fifo_start(mpu6050_t *m, uint8_t sources);   /* reset, select sources, enable */
int mpu6050_fifo_stop(mpu6050_t *m);
int mpu6050_fifo_reset(mpu6050_t *m);
int mpu6050_fifo_count(mpu6050_t *m, uint16_t *bytes);

/* Read up to max_samples whole samples. Returns EDEV_OK and sets *got.
 * The sample layout follows the sources chosen in fifo_start. */
int mpu6050_fifo_read(mpu6050_t *m, mpu6050_raw_t *out, uint16_t max_samples, uint16_t *got);

typedef enum {
    MPU6050_CLK_INTERNAL = 0,   /* 8 MHz internal oscillator (less stable) */
    MPU6050_CLK_PLL_X    = 1,   /* gyro X PLL (recommended, the default) */
    MPU6050_CLK_PLL_Y    = 2,
    MPU6050_CLK_PLL_Z    = 3,
    MPU6050_CLK_STOP     = 7    /* clock stopped, timing generator in reset */
} mpu6050_clock_t;

/* Wake-up rate in cycle mode (approximate on MPU6500). */
typedef enum {
    MPU6050_LP_1_25HZ = 0,
    MPU6050_LP_5HZ    = 1,
    MPU6050_LP_20HZ   = 2,
    MPU6050_LP_40HZ   = 3
} mpu6050_lp_rate_t;

int mpu6050_set_clock   (mpu6050_t *m, mpu6050_clock_t clk);
int mpu6050_temp_enable (mpu6050_t *m, bool enable);

/* mask = OR of MPU6050_PWR2_STBY_XA/YA/ZA/XG/YG/ZG. Bits set = axis in standby. */
int mpu6050_standby     (mpu6050_t *m, uint8_t mask);

/* Low-power accelerometer-only mode: gyro and temperature off,
 * the chip wakes at 'rate', takes an accel sample, and sleeps. */
int mpu6050_cycle_start (mpu6050_t *m, mpu6050_lp_rate_t rate);
int mpu6050_cycle_stop  (mpu6050_t *m);

#endif /* EDEV_MPU6050_H */