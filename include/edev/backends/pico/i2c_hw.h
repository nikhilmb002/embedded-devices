#ifndef EDEV_PICO_I2C_HW_H
#define EDEV_PICO_I2C_HW_H

#include <stdint.h>
#include "edev/bus/bus_i2c.h"
#include "hardware/i2c.h"

/*
 * Initialise a Pico hardware I2C bus.
 *   inst : i2c0 or i2c1
 *   sda, scl : GPIO numbers (must be valid I2C pins for that instance)
 *   hz : bus speed, e.g. 100000 or 400000
 */
int edev_pico_i2c_init(edev_i2c_bus_t *bus, i2c_inst_t *inst,
                       uint sda, uint scl, uint32_t hz);

#endif /* EDEV_PICO_I2C_HW_H */
