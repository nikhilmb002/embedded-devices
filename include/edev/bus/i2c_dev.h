#ifndef EDEV_I2C_DEV_H
#define EDEV_I2C_DEV_H

#include "edev/bus/bus_i2c.h"

// A device on an I2C bus: which bus + which 7-bit address.
typedef struct {
    edev_i2c_bus_t *bus;
    uint8_t         addr;
} edev_i2c_dev_t;

void edev_i2c_dev_init(edev_i2c_dev_t *dev, edev_i2c_bus_t *bus, uint8_t addr);

// Raw write/read to the device
int edev_i2c_dev_write(edev_i2c_dev_t *dev, const uint8_t *buf, size_t len);
int edev_i2c_dev_read (edev_i2c_dev_t *dev, uint8_t *buf, size_t len);

// Register helpers: [reg, value] write, and reg -> repeated START -> read.
int edev_i2c_dev_write_reg(edev_i2c_dev_t *dev, uint8_t reg, uint8_t value);
int edev_i2c_dev_read_regs(edev_i2c_dev_t *dev, uint8_t reg, uint8_t *buf, size_t len);

// EDEV_OK if the device ACKs its address
int edev_i2c_dev_probe(edev_i2c_dev_t *dev);

#endif /* EDEV_I2C_DEV_H */