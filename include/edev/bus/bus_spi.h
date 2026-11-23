#ifndef EDEV_BUS_SPI_H
#define EDEV_BUS_SPI_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "edev/edev_errors.h"

typedef struct edev_spi_bus edev_spi_bus_t;

/*
 * Operations a backend must provide.
 *
 * transfer: full-duplex shift of 'len' bytes.
 *   tx==NULL → send 'repeated_tx' while reading into rx.
 *   rx==NULL → discard received bytes while sending tx.
 *   Both non-NULL → simultaneous write/read.
 *
 * cs_select / cs_deselect: drive the chip-select pin low / high.
 */
typedef struct {
    int  (*transfer)(edev_spi_bus_t *bus,
                     const uint8_t *tx, uint8_t *rx, size_t len,
                     uint8_t repeated_tx);
    void (*cs_select)(edev_spi_bus_t *bus, unsigned int cs_pin);
    void (*cs_deselect)(edev_spi_bus_t *bus, unsigned int cs_pin);
} edev_spi_ops_t;

struct edev_spi_bus {
    const edev_spi_ops_t *ops;
    void                 *ctx;   /* backend private data (e.g. spi0) */
};

static inline int edev_spi_transfer(edev_spi_bus_t *bus,
                                     const uint8_t *tx, uint8_t *rx,
                                     size_t len, uint8_t repeated_tx) {
    if (!bus || !bus->ops || !bus->ops->transfer) return EDEV_ERR_INVAL;
    if (len == 0) return EDEV_OK;
    return bus->ops->transfer(bus, tx, rx, len, repeated_tx);
}

static inline void edev_spi_cs_select(edev_spi_bus_t *bus, unsigned int cs_pin) {
    if (bus && bus->ops && bus->ops->cs_select)
        bus->ops->cs_select(bus, cs_pin);
}

static inline void edev_spi_cs_deselect(edev_spi_bus_t *bus, unsigned int cs_pin) {
    if (bus && bus->ops && bus->ops->cs_deselect)
        bus->ops->cs_deselect(bus, cs_pin);
}

#endif /* EDEV_BUS_SPI_H */
