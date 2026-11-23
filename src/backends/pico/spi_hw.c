#include "edev/backends/pico/spi_hw.h"
#include "pico/stdlib.h"

static int pico_spi_transfer(edev_spi_bus_t *bus,
                              const uint8_t *tx, uint8_t *rx, size_t len,
                              uint8_t repeated_tx) {

    spi_inst_t *spi = (spi_inst_t *)bus->ctx;
    int r;

    if (tx && rx)
        r = spi_write_read_blocking(spi, tx, rx, len);
    else if (tx)
        r = spi_write_blocking(spi, tx, len);
    else if (rx)
        r = spi_read_blocking(spi, repeated_tx, rx, len);
    else
        return EDEV_ERR_INVAL;

    return (r == (int)len) ? EDEV_OK : EDEV_ERR_IO;
}

static void pico_cs_select(edev_spi_bus_t *bus, unsigned int cs_pin) {

    (void)bus;
    gpio_put(cs_pin, 0);
}

static void pico_cs_deselect(edev_spi_bus_t *bus, unsigned int cs_pin) {

    (void)bus;
    gpio_put(cs_pin, 1);
}

static const edev_spi_ops_t pico_spi_ops = {
    .transfer    = pico_spi_transfer,
    .cs_select   = pico_cs_select,
    .cs_deselect = pico_cs_deselect,
};

int edev_pico_spi_init(edev_spi_bus_t *bus, spi_inst_t *inst,
                       unsigned int miso, unsigned int mosi,
                       unsigned int sck, uint32_t hz) {

    if (!bus || !inst) return EDEV_ERR_INVAL;

    spi_init(inst, hz);
    gpio_set_function(miso, GPIO_FUNC_SPI);
    gpio_set_function(mosi, GPIO_FUNC_SPI);
    gpio_set_function(sck,  GPIO_FUNC_SPI);

    bus->ops = &pico_spi_ops;
    bus->ctx = inst;
    return EDEV_OK;
}
