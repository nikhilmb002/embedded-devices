#include "edev/drivers/flash/w25q32.h"
#include <string.h>

/* ---- W25Q32 SPI instruction opcodes ---- */

#define CMD_WRITE_ENABLE     0x06
#define CMD_WRITE_DISABLE    0x04
#define CMD_READ_STATUS_1    0x05
#define CMD_READ_DATA        0x03
#define CMD_PAGE_PROGRAM     0x02
#define CMD_SECTOR_ERASE     0x20
#define CMD_BLOCK_ERASE_32K  0x52
#define CMD_BLOCK_ERASE_64K  0xD8
#define CMD_CHIP_ERASE       0xC7
#define CMD_POWER_DOWN       0xB9
#define CMD_RELEASE_PD       0xAB
#define CMD_JEDEC_ID         0x9F

#define SR1_BUSY  0x01
#define SR1_WEL   0x02

/* ---- Internal helpers ---- */

static int write_enable(w25q32_t *dev) {
    uint8_t cmd = CMD_WRITE_ENABLE;
    return edev_spi_dev_write(&dev->dev, &cmd, 1);
}

/* Busy-wait: poll SR1 until BUSY clears or max_loops exceeded. */
static int wait_busy(w25q32_t *dev, uint32_t max_loops) {

    uint8_t cmd = CMD_READ_STATUS_1;
    uint8_t sr;

    for (uint32_t i = 0; i < max_loops; i++) {
        int r = edev_spi_dev_write_then_read(&dev->dev, &cmd, 1, &sr, 1);
        if (r != EDEV_OK) return r;
        if (!(sr & SR1_BUSY)) return EDEV_OK;
    }
    return EDEV_ERR_TIMEOUT;
}

static int erase_cmd(w25q32_t *dev, uint8_t opcode, uint32_t addr,
                     uint32_t timeout_loops) {

    int r = write_enable(dev);
    if (r != EDEV_OK) return r;

    uint8_t cmd[4] = {
        opcode,
        (uint8_t)(addr >> 16),
        (uint8_t)(addr >> 8),
        (uint8_t)(addr)
    };
    r = edev_spi_dev_write(&dev->dev, cmd, 4);
    if (r != EDEV_OK) return r;

    return wait_busy(dev, timeout_loops);
}

/* ---- Public API ---- */

int w25q32_read_status(w25q32_t *dev, uint8_t *sr1) {

    if (!dev || !sr1) return EDEV_ERR_INVAL;
    uint8_t cmd = CMD_READ_STATUS_1;
    return edev_spi_dev_write_then_read(&dev->dev, &cmd, 1, sr1, 1);
}

bool w25q32_is_busy(w25q32_t *dev) {

    uint8_t sr = 0xFF;
    w25q32_read_status(dev, &sr);
    return (sr & SR1_BUSY) != 0;
}

int w25q32_read_jedec_id(w25q32_t *dev, w25q32_jedec_id_t *id) {

    if (!dev || !id) return EDEV_ERR_INVAL;

    uint8_t cmd = CMD_JEDEC_ID;
    uint8_t buf[3];
    int r = edev_spi_dev_write_then_read(&dev->dev, &cmd, 1, buf, 3);
    if (r != EDEV_OK) return r;

    id->manufacturer = buf[0];
    id->mem_type     = buf[1];
    id->capacity     = buf[2];
    return EDEV_OK;
}

int w25q32_init(w25q32_t *dev, edev_spi_bus_t *bus, unsigned int cs_pin) {

    if (!dev || !bus) return EDEV_ERR_INVAL;

    edev_spi_dev_init(&dev->dev, bus, cs_pin);

    /* Verify JEDEC ID matches a W25Q32 */
    w25q32_jedec_id_t id;
    int r = w25q32_read_jedec_id(dev, &id);
    if (r != EDEV_OK) return r;

    if (id.manufacturer != W25Q32_JEDEC_MFR ||
        id.mem_type     != W25Q32_JEDEC_TYPE ||
        id.capacity     != W25Q32_JEDEC_CAPACITY)
        return EDEV_ERR_IO;   /* wrong or missing chip */

    return EDEV_OK;
}

int w25q32_read(w25q32_t *dev, uint32_t addr, uint8_t *buf, size_t len) {

    if (!dev || !buf || len == 0) return EDEV_ERR_INVAL;
    if (addr + len > W25Q32_TOTAL_SIZE) return EDEV_ERR_INVAL;

    uint8_t cmd[4] = {
        CMD_READ_DATA,
        (uint8_t)(addr >> 16),
        (uint8_t)(addr >> 8),
        (uint8_t)(addr)
    };
    return edev_spi_dev_write_then_read(&dev->dev, cmd, 4, buf, len);
}

int w25q32_page_program(w25q32_t *dev, uint32_t addr,
                        const uint8_t *data, size_t len) {

    if (!dev || !data || len == 0 || len > W25Q32_PAGE_SIZE)
        return EDEV_ERR_INVAL;
    if (addr + len > W25Q32_TOTAL_SIZE) return EDEV_ERR_INVAL;

    /* Data must not cross a 256-byte page boundary */
    if ((addr / W25Q32_PAGE_SIZE) != ((addr + len - 1) / W25Q32_PAGE_SIZE))
        return EDEV_ERR_INVAL;

    int r = write_enable(dev);
    if (r != EDEV_OK) return r;

    /* Send header + data in one CS assertion */
    uint8_t hdr[4] = {
        CMD_PAGE_PROGRAM,
        (uint8_t)(addr >> 16),
        (uint8_t)(addr >> 8),
        (uint8_t)(addr)
    };

    edev_spi_cs_select(dev->dev.bus, dev->dev.cs_pin);
    r = edev_spi_transfer(dev->dev.bus, hdr, NULL, 4, 0x00);
    if (r == EDEV_OK)
        r = edev_spi_transfer(dev->dev.bus, data, NULL, len, 0x00);
    edev_spi_cs_deselect(dev->dev.bus, dev->dev.cs_pin);

    if (r != EDEV_OK) return r;

    return wait_busy(dev, 50000);   /* ~5 ms worth of polls */
}

int w25q32_write(w25q32_t *dev, uint32_t addr,
                 const uint8_t *data, size_t len) {

    if (!dev || !data || len == 0) return EDEV_ERR_INVAL;
    if (addr + len > W25Q32_TOTAL_SIZE) return EDEV_ERR_INVAL;

    size_t offset = 0;
    while (offset < len) {
        /* How many bytes until end of current 256-byte page? */
        size_t page_remaining = W25Q32_PAGE_SIZE -
                                (addr % W25Q32_PAGE_SIZE);
        size_t chunk = (len - offset < page_remaining)
                     ? (len - offset) : page_remaining;

        int r = w25q32_page_program(dev, addr, data + offset, chunk);
        if (r != EDEV_OK) return r;

        addr   += (uint32_t)chunk;
        offset += chunk;
    }
    return EDEV_OK;
}

int w25q32_erase_sector(w25q32_t *dev, uint32_t addr) {

    if (!dev || addr >= W25Q32_TOTAL_SIZE) return EDEV_ERR_INVAL;
    return erase_cmd(dev, CMD_SECTOR_ERASE, addr, 500000);
}

int w25q32_erase_block_32k(w25q32_t *dev, uint32_t addr) {

    if (!dev || addr >= W25Q32_TOTAL_SIZE) return EDEV_ERR_INVAL;
    return erase_cmd(dev, CMD_BLOCK_ERASE_32K, addr, 1600000);
}

int w25q32_erase_block_64k(w25q32_t *dev, uint32_t addr) {

    if (!dev || addr >= W25Q32_TOTAL_SIZE) return EDEV_ERR_INVAL;
    return erase_cmd(dev, CMD_BLOCK_ERASE_64K, addr, 2000000);
}

int w25q32_erase_chip(w25q32_t *dev) {

    if (!dev) return EDEV_ERR_INVAL;

    int r = write_enable(dev);
    if (r != EDEV_OK) return r;

    uint8_t cmd = CMD_CHIP_ERASE;
    r = edev_spi_dev_write(&dev->dev, &cmd, 1);
    if (r != EDEV_OK) return r;

    return wait_busy(dev, 200000000);
}

int w25q32_power_down(w25q32_t *dev) {

    if (!dev) return EDEV_ERR_INVAL;
    uint8_t cmd = CMD_POWER_DOWN;
    return edev_spi_dev_write(&dev->dev, &cmd, 1);
}

int w25q32_power_up(w25q32_t *dev) {

    if (!dev) return EDEV_ERR_INVAL;
    uint8_t cmd = CMD_RELEASE_PD;
    return edev_spi_dev_write(&dev->dev, &cmd, 1);
}
