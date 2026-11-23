#ifndef EDEV_PICO_SPI_HW_H
#define EDEV_PICO_SPI_HW_H

#include <stdint.h>
#include "edev/bus/bus_spi.h"
#include "hardware/spi.h"

/*
 * Initialise a Pico hardware SPI bus.
 *   inst       : spi0 or spi1
 *   miso, mosi, sck : GPIO numbers (must be valid SPI pins for that instance)
 *   hz         : bus speed, e.g. 1000000 (1 MHz)
 *
 * CS pins are NOT configured here — each spi_dev owns its own CS pin,
 * which the application must set up as a GPIO output before use.
 */
int edev_pico_spi_init(edev_spi_bus_t *bus, spi_inst_t *inst,
                       unsigned int miso, unsigned int mosi,
                       unsigned int sck, uint32_t hz);

#endif /* EDEV_PICO_SPI_HW_H */
