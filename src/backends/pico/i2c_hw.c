#include "edev/backends/pico/i2c_hw.h"
#include "pico/stdlib.h"

#define EDEV_PICO_I2C_TIMEOUT_US 50000   /* 50 ms per transfer */

/* Translate Pico SDK return values into edev error codes. */
static int convert(int r, size_t len) {
    
    if (r == (int)len)           return EDEV_OK;
    if (r == PICO_ERROR_TIMEOUT) return EDEV_ERR_TIMEOUT;
    return EDEV_ERR_NACK;        /* PICO_ERROR_GENERIC = NACK or bus error */
}

static int pico_write(edev_i2c_bus_t *bus, uint8_t addr,
                      const uint8_t *buf, size_t len, bool nostop) {
    
    int r = i2c_write_timeout_us((i2c_inst_t *)bus->ctx, addr, buf, len,
                                 nostop, EDEV_PICO_I2C_TIMEOUT_US);
    return convert(r, len);
}

static int pico_read(edev_i2c_bus_t *bus, uint8_t addr,
                     uint8_t *buf, size_t len, bool nostop) {

    int r = i2c_read_timeout_us((i2c_inst_t *)bus->ctx, addr, buf, len,
                                nostop, EDEV_PICO_I2C_TIMEOUT_US);
    return convert(r, len);
}

static const edev_i2c_ops_t pico_ops = {
    .write = pico_write,
    .read  = pico_read,
};

int edev_pico_i2c_init(edev_i2c_bus_t *bus, i2c_inst_t *inst,
                       uint sda, uint scl, uint32_t hz) {
                        
    if (!bus || !inst) return EDEV_ERR_INVAL;

    i2c_init(inst, hz);
    gpio_set_function(sda, GPIO_FUNC_I2C);
    gpio_set_function(scl, GPIO_FUNC_I2C);
    gpio_pull_up(sda);    /* weak backup, real pull-ups are on the module */
    gpio_pull_up(scl);

    bus->ops = &pico_ops;     /* plug our functions into the table */
    bus->ctx = inst;          /* remember which I2C block we use */
    return EDEV_OK;
}