#ifndef EDEV_W25Q32_H
#define EDEV_W25Q32_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "edev/bus/spi_dev.h"

/* W25Q32: 32 Mbit = 4 MB */
#define W25Q32_TOTAL_SIZE       (4u * 1024u * 1024u)   /* 4 194 304 bytes */
#define W25Q32_PAGE_SIZE        256u
#define W25Q32_SECTOR_SIZE      (4u * 1024u)            /* 4 KB */
#define W25Q32_BLOCK_32K_SIZE   (32u * 1024u)
#define W25Q32_BLOCK_64K_SIZE   (64u * 1024u)
#define W25Q32_PAGE_COUNT       (W25Q32_TOTAL_SIZE / W25Q32_PAGE_SIZE)
#define W25Q32_SECTOR_COUNT     (W25Q32_TOTAL_SIZE / W25Q32_SECTOR_SIZE)

/* Expected JEDEC ID for W25Q32JV */
#define W25Q32_JEDEC_MFR        0xEF   /* Winbond */
#define W25Q32_JEDEC_TYPE       0x40
#define W25Q32_JEDEC_CAPACITY   0x16   /* 32 Mbit */

/*
 * The caller owns this struct (declare it static or global).
 * No hidden globals, so you can have several flash chips.
 */
typedef struct {
    edev_spi_dev_t dev;
} w25q32_t;

/* JEDEC identification triplet. */
typedef struct {
    uint8_t manufacturer;   /* 0xEF for Winbond */
    uint8_t mem_type;       /* 0x40 */
    uint8_t capacity;       /* 0x16 for 32 Mbit */
} w25q32_jedec_id_t;

/* Initialise: set up the SPI device and validate the JEDEC ID. */
int w25q32_init(w25q32_t *dev, edev_spi_bus_t *bus, unsigned int cs_pin);

/* Read the JEDEC ID without full init. */
int w25q32_read_jedec_id(w25q32_t *dev, w25q32_jedec_id_t *id);

/* Status register (SR1). */
int  w25q32_read_status(w25q32_t *dev, uint8_t *sr1);
bool w25q32_is_busy(w25q32_t *dev);

/* Read data from any address, any length.
 * addr + len must not exceed W25Q32_TOTAL_SIZE. */
int w25q32_read(w25q32_t *dev, uint32_t addr, uint8_t *buf, size_t len);

/* Page program (up to 256 bytes within one page boundary).
 * Waits for completion (BUSY bit) before returning. */
int w25q32_page_program(w25q32_t *dev, uint32_t addr,
                        const uint8_t *data, size_t len);

/* Write arbitrary length across page boundaries (auto-paging).
 * Waits for each page to complete before continuing. */
int w25q32_write(w25q32_t *dev, uint32_t addr,
                 const uint8_t *data, size_t len);

/* Erase operations — all wait for completion before returning. */
int w25q32_erase_sector(w25q32_t *dev, uint32_t addr);     /* 4 KB */
int w25q32_erase_block_32k(w25q32_t *dev, uint32_t addr);  /* 32 KB */
int w25q32_erase_block_64k(w25q32_t *dev, uint32_t addr);  /* 64 KB */
int w25q32_erase_chip(w25q32_t *dev);                       /* entire chip */

/* Power management. */
int w25q32_power_down(w25q32_t *dev);
int w25q32_power_up(w25q32_t *dev);

#endif /* EDEV_W25Q32_H */
