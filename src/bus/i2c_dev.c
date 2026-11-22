#include "edev/bus/i2c_dev.h"

void edev_i2c_dev_init(edev_i2c_dev_t *dev, edev_i2c_bus_t *bus, uint8_t addr) {
    
    dev->bus  = bus;
    dev->addr = addr;
}

int edev_i2c_dev_write(edev_i2c_dev_t *dev, const uint8_t *buf, size_t len) {
   
    if (!dev) return EDEV_ERR_INVAL;
    return edev_i2c_write(dev->bus, dev->addr, buf, len, false);
}

int edev_i2c_dev_read(edev_i2c_dev_t *dev, uint8_t *buf, size_t len) {
   
    if (!dev) return EDEV_ERR_INVAL;
    return edev_i2c_read(dev->bus, dev->addr, buf, len, false);
}

int edev_i2c_dev_write_reg(edev_i2c_dev_t *dev, uint8_t reg, uint8_t value) {
  
    if (!dev) return EDEV_ERR_INVAL;
    uint8_t buf[2] = { reg, value };
    return edev_i2c_write(dev->bus, dev->addr, buf, 2, false);
}

int edev_i2c_dev_read_regs(edev_i2c_dev_t *dev, uint8_t reg, uint8_t *buf, size_t len) {
   
    if (!dev) return EDEV_ERR_INVAL;
    int r = edev_i2c_write(dev->bus, dev->addr, &reg, 1, true);  /* no STOP */
    if (r != EDEV_OK) return r;
    return edev_i2c_read(dev->bus, dev->addr, buf, len, false);  /* repeated START */
}

int edev_i2c_dev_probe(edev_i2c_dev_t *dev) {
    
    if (!dev) return EDEV_ERR_INVAL;
    uint8_t dummy;
    return edev_i2c_read(dev->bus, dev->addr, &dummy, 1, false);
}

