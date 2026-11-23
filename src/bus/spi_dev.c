#include "edev/bus/spi_dev.h"

void edev_spi_dev_init(edev_spi_dev_t *dev, edev_spi_bus_t *bus,
                       unsigned int cs_pin) {
    dev->bus    = bus;
    dev->cs_pin = cs_pin;
}

int edev_spi_dev_write(edev_spi_dev_t *dev, const uint8_t *buf, size_t len) {

    if (!dev || !buf || len == 0) return EDEV_ERR_INVAL;
    edev_spi_cs_select(dev->bus, dev->cs_pin);
    int r = edev_spi_transfer(dev->bus, buf, NULL, len, 0x00);
    edev_spi_cs_deselect(dev->bus, dev->cs_pin);
    return r;
}

int edev_spi_dev_write_then_read(edev_spi_dev_t *dev,
                                  const uint8_t *tx, size_t tx_len,
                                  uint8_t *rx, size_t rx_len) {

    if (!dev) return EDEV_ERR_INVAL;
    edev_spi_cs_select(dev->bus, dev->cs_pin);
    int r = EDEV_OK;
    if (tx && tx_len > 0)
        r = edev_spi_transfer(dev->bus, tx, NULL, tx_len, 0x00);
    if (r == EDEV_OK && rx && rx_len > 0)
        r = edev_spi_transfer(dev->bus, NULL, rx, rx_len, 0x00);
    edev_spi_cs_deselect(dev->bus, dev->cs_pin);
    return r;
}

int edev_spi_dev_transfer(edev_spi_dev_t *dev,
                           const uint8_t *tx, uint8_t *rx, size_t len) {

    if (!dev) return EDEV_ERR_INVAL;
    edev_spi_cs_select(dev->bus, dev->cs_pin);
    int r = edev_spi_transfer(dev->bus, tx, rx, len, 0x00);
    edev_spi_cs_deselect(dev->bus, dev->cs_pin);
    return r;
}
