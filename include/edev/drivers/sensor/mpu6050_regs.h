#ifndef EDEV_MPU6050_REGS_H
#define EDEV_MPU6050_REGS_H

/* MPU-6000/6050 register map (from the InvenSense register map document). */

/* I2C address: AD0 pin low = 0x68, high = 0x69 */
#define MPU6050_ADDR_AD0_LOW   0x68
#define MPU6050_ADDR_AD0_HIGH  0x69
#define MPU6050_WHO_AM_I_VALUE 0x68      /* the chip returns 0x68 regardless of AD0 */

/* ---- Registers ---- */
#define MPU6050_REG_SELF_TEST_X      0x0D
#define MPU6050_REG_SELF_TEST_Y      0x0E
#define MPU6050_REG_SELF_TEST_Z      0x0F
#define MPU6050_REG_SELF_TEST_A      0x10

#define MPU6050_REG_XA_OFFS_H        0x06   /* accel offsets (factory-trimmed, advanced use) */
#define MPU6050_REG_XG_OFFS_USRH     0x13   /* gyro offsets: XH XL YH YL ZH ZL */

#define MPU6050_REG_SMPLRT_DIV       0x19
#define MPU6050_REG_CONFIG           0x1A
#define MPU6050_REG_GYRO_CONFIG      0x1B
#define MPU6050_REG_ACCEL_CONFIG     0x1C
#define MPU6050_REG_FF_THR           0x1D
#define MPU6050_REG_FF_DUR           0x1E
#define MPU6050_REG_MOT_THR          0x1F
#define MPU6050_REG_MOT_DUR          0x20
#define MPU6050_REG_ZRMOT_THR        0x21
#define MPU6050_REG_ZRMOT_DUR        0x22
#define MPU6050_REG_FIFO_EN          0x23

#define MPU6050_REG_I2C_MST_CTRL     0x24
#define MPU6050_REG_I2C_SLV0_ADDR    0x25
#define MPU6050_REG_I2C_SLV0_REG     0x26
#define MPU6050_REG_I2C_SLV0_CTRL    0x27
#define MPU6050_REG_I2C_MST_STATUS   0x36

#define MPU6050_REG_INT_PIN_CFG      0x37
#define MPU6050_REG_INT_ENABLE       0x38
#define MPU6050_REG_INT_STATUS       0x3A

#define MPU6050_REG_ACCEL_XOUT_H     0x3B   /* 14 bytes: AX AY AZ (6), TEMP (2), GX GY GZ (6) */
#define MPU6050_REG_TEMP_OUT_H       0x41
#define MPU6050_REG_GYRO_XOUT_H      0x43
#define MPU6050_REG_EXT_SENS_DATA_00 0x49

#define MPU6050_REG_MOT_DETECT_STATUS 0x61
#define MPU6050_REG_I2C_MST_DELAY_CTRL 0x67
#define MPU6050_REG_SIGNAL_PATH_RESET 0x68
#define MPU6050_REG_MOT_DETECT_CTRL  0x69
#define MPU6050_REG_USER_CTRL        0x6A
#define MPU6050_REG_PWR_MGMT_1       0x6B
#define MPU6050_REG_PWR_MGMT_2       0x6C
#define MPU6050_REG_FIFO_COUNT_H     0x72
#define MPU6050_REG_FIFO_COUNT_L     0x73
#define MPU6050_REG_FIFO_R_W         0x74
#define MPU6050_REG_WHO_AM_I         0x75

/* ---- CONFIG (0x1A) ---- */
#define MPU6050_DLPF_CFG_MASK        0x07   /* 0 = 260 Hz (gyro 8 kHz) ... 6 = 5 Hz */

/* ---- GYRO_CONFIG (0x1B) / ACCEL_CONFIG (0x1C) ---- */
#define MPU6050_FS_SEL_SHIFT         3      /* full-scale range in bits 4:3 */
#define MPU6050_FS_SEL_MASK          (0x03 << MPU6050_FS_SEL_SHIFT)
#define MPU6050_SELFTEST_X           0x80
#define MPU6050_SELFTEST_Y           0x40
#define MPU6050_SELFTEST_Z           0x20
#define MPU6050_ACCEL_HPF_MASK       0x07

/* ---- FIFO_EN (0x23): which readings go into the FIFO ---- */
#define MPU6050_FIFO_TEMP            0x80
#define MPU6050_FIFO_GYRO_X          0x40
#define MPU6050_FIFO_GYRO_Y          0x20
#define MPU6050_FIFO_GYRO_Z          0x10
#define MPU6050_FIFO_ACCEL           0x08   /* all three accel axes */

/* ---- INT_PIN_CFG (0x37) ---- */
#define MPU6050_INT_LEVEL_LOW        0x80   /* 1 = INT pin active low */
#define MPU6050_INT_OPEN_DRAIN       0x40
#define MPU6050_INT_LATCH_EN         0x20   /* hold INT until status is read */
#define MPU6050_INT_RD_CLEAR         0x10   /* any read clears INT_STATUS */
#define MPU6050_I2C_BYPASS_EN        0x02   /* aux bus connected to the main bus */

/* ---- INT_ENABLE (0x38) and INT_STATUS (0x3A) ---- */
#define MPU6050_INT_FF               0x80   /* free fall */
#define MPU6050_INT_MOT              0x40   /* motion */
#define MPU6050_INT_ZMOT             0x20   /* zero motion */
#define MPU6050_INT_FIFO_OFLOW       0x10
#define MPU6050_INT_I2C_MST          0x08
#define MPU6050_INT_DATA_RDY         0x01

/* ---- USER_CTRL (0x6A) ---- */
#define MPU6050_USER_FIFO_EN         0x40
#define MPU6050_USER_I2C_MST_EN      0x20
#define MPU6050_USER_FIFO_RESET      0x04
#define MPU6050_USER_I2C_MST_RESET   0x02
#define MPU6050_USER_SIG_COND_RESET  0x01

/* ---- PWR_MGMT_1 (0x6B) ---- */
#define MPU6050_PWR1_DEVICE_RESET    0x80
#define MPU6050_PWR1_SLEEP           0x40   /* set after power-up: chip starts asleep */
#define MPU6050_PWR1_CYCLE           0x20   /* wake-sample-sleep cycling */
#define MPU6050_PWR1_TEMP_DIS        0x08
#define MPU6050_PWR1_CLKSEL_MASK     0x07   /* 0 = internal 8 MHz, 1 = PLL gyro X (recommended) */

/* ---- PWR_MGMT_2 (0x6C) ---- */
#define MPU6050_PWR2_LP_WAKE_SHIFT   6      /* cycle wake-up frequency: 1.25/5/20/40 Hz */
#define MPU6050_PWR2_STBY_XA         0x20
#define MPU6050_PWR2_STBY_YA         0x10
#define MPU6050_PWR2_STBY_ZA         0x08
#define MPU6050_PWR2_STBY_XG         0x04
#define MPU6050_PWR2_STBY_YG         0x02
#define MPU6050_PWR2_STBY_ZG         0x01

/* ---- Sensitivities (LSB per unit) and temperature formula ---- */
/* accel: 16384, 8192, 4096, 2048 LSB/g for +-2, 4, 8, 16 g
 * gyro : 131, 65.5, 32.8, 16.4 LSB/(deg/s) for +-250, 500, 1000, 2000
 * temp : degC = raw / 340 + 36.53 */


/* MPU6500-only registers (clone support) */
#define MPU6500_REG_ACCEL_CONFIG2   0x1D
#define MPU6500_REG_LP_ACCEL_ODR    0x1E
#define MPU6500_ACCEL_FCHOICE_B     0x08

#endif /* EDEV_MPU6050_REGS_H */