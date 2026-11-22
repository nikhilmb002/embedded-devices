#ifndef EDEV_BUS_I2C_H
#define EDEV_BUS_I2C_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "edev/edev_errors.h"

typedef struct edev_i2c_bus edev_i2c_bus_t;

/* 
     * Operations a backend must provide 
     * nostop = true: do not send STOP after this transfer, so the next 
     * transfer begins with a repeated START (used fr register reads).
     * Both functions return EDEV_OK or a negative EDEV_ERR_* code.
*/

typedef struct {
  
    int (*write)(edev_i2c_bus_t *bus, uint8_t addr,
                 const uint8_t *buf, size_t len, bool nostop);
    int (*read) (edev_i2c_bus_t *bus, uint8_t addr,
                 uint8_t *buf, size_t len, bool nostop);
} edev_i2c_ops_t;

struct edev_i2c_bus {
    
    const edev_i2c_ops_t *ops;   /* set by the backend */
    void *ctx;                   /* backend private data (e.g. i2c0) */
};

static inline int edev_i2c_read(edev_i2c_bus_t *bus, uint8_t addr,
                                uint8_t *buf, size_t len, bool nostop) {
    
    if (!bus || !bus->ops || !buf || len == 0) return EDEV_ERR_INVAL;
    if (!bus->ops->read) return EDEV_ERR_NOSUPPORT;
    return bus->ops->read(bus, addr, buf, len, nostop);
}

#endif /* EDEV_BUS_I2C_H */