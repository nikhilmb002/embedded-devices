#ifndef EDEV_SPI_DEV_H
#define EDEV_SPI_DEV_H

#include "edev/bus/bus_spi.h"

/* A device on an SPI bus: which bus + which chip-select pin. */
typedef struct {
    edev_spi_bus_t *bus;
    unsigned int    cs_pin;
} edev_spi_dev_t;

void edev_spi_dev_init(edev_spi_dev_t *dev, edev_spi_bus_t *bus,
                       unsigned int cs_pin);

/* Select CS, write buf, deselect CS. */
int edev_spi_dev_write(edev_spi_dev_t *dev, const uint8_t *buf, size_t len);

/* Select CS, write tx (command/addr), then read into rx, deselect CS.
 * Sends 0x00 as repeated_tx during the read phase. */
int edev_spi_dev_write_then_read(edev_spi_dev_t *dev,
                                  const uint8_t *tx, size_t tx_len,
                                  uint8_t *rx, size_t rx_len);

/* Select CS, full-duplex transfer, deselect CS. */
int edev_spi_dev_transfer(edev_spi_dev_t *dev,
                           const uint8_t *tx, uint8_t *rx, size_t len);

#endif /* EDEV_SPI_DEV_H */
