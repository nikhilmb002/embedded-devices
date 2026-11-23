/* W25Q32 flash demo: JEDEC ID, erase, write, read-back, cross-page, power. */
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "edev/backends/pico/spi_hw.h"
#include "edev/drivers/flash/w25q32.h"

/*
 * Default wiring (see docs/wiring/pico2_w25q32.md):
 *   MISO = GP16, CS = GP17, SCK = GP18, MOSI = GP19
 *   VCC = 3.3V, GND = GND, /WP = 3.3V, /HOLD = 3.3V
 */
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

static edev_spi_bus_t bus;
static w25q32_t       flash;

static void test_jedec_id(void) {

    w25q32_jedec_id_t id;
    int r = w25q32_read_jedec_id(&flash, &id);
    printf("JEDEC ID: mfr=0x%02X type=0x%02X cap=0x%02X (r=%d)\n",
           id.manufacturer, id.mem_type, id.capacity, r);
}

static void test_read_write(void) {

    const uint32_t addr = 0x000000;   /* first sector */

    /* 1. Erase sector */
    printf("Erasing sector 0... ");
    int r = w25q32_erase_sector(&flash, addr);
    printf("r=%d\n", r);

    /* 2. Verify erased (all 0xFF) */
    uint8_t buf[256];
    r = w25q32_read(&flash, addr, buf, sizeof(buf));
    bool erased = true;
    for (int i = 0; i < 256; i++) {
        if (buf[i] != 0xFF) { erased = false; break; }
    }
    printf("Sector erased: %s (r=%d)\n", erased ? "YES" : "NO", r);

    /* 3. Write a test pattern */
    uint8_t pattern[64];
    for (int i = 0; i < 64; i++) pattern[i] = (uint8_t)i;
    printf("Writing 64 bytes... ");
    r = w25q32_write(&flash, addr, pattern, 64);
    printf("r=%d\n", r);

    /* 4. Read back and verify */
    memset(buf, 0, 64);
    r = w25q32_read(&flash, addr, buf, 64);
    bool match = (memcmp(buf, pattern, 64) == 0);
    printf("Readback match: %s (r=%d)\n", match ? "YES" : "NO", r);

    /* 5. Cross-page write test: start at 0xF0 (page 0, offset 240)
     *    and write 32 bytes, which spans into page 1. */
    printf("Cross-page write test (addr=0xF0, 32 bytes)... ");
    r = w25q32_erase_sector(&flash, 0);
    if (r == EDEV_OK)
        r = w25q32_write(&flash, 0xF0, pattern, 32);
    printf("r=%d\n", r);

    memset(buf, 0, 32);
    r = w25q32_read(&flash, 0xF0, buf, 32);
    match = (memcmp(buf, pattern, 32) == 0);
    printf("Cross-page readback: %s (r=%d)\n", match ? "YES" : "NO", r);
}

static void test_power_management(void) {

    printf("Power down... ");
    int r = w25q32_power_down(&flash);
    printf("r=%d\n", r);

    sleep_ms(10);

    printf("Power up... ");
    r = w25q32_power_up(&flash);
    printf("r=%d\n", r);

    sleep_ms(1);   /* tRES1 release time */

    /* Verify chip responds after wakeup */
    w25q32_jedec_id_t id;
    r = w25q32_read_jedec_id(&flash, &id);
    printf("After wakeup: mfr=0x%02X (r=%d)\n", id.manufacturer, r);
}

int main(void) {

    stdio_init_all();
    sleep_ms(2000);

    printf("\n=== W25Q32 Flash Demo ===\n\n");

    /* Init SPI bus */
    edev_pico_spi_init(&bus, spi0, PIN_MISO, PIN_MOSI, PIN_SCK, 1000000);

    /* CS pin setup: GPIO, output, deselected (high) */
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_DIR_OUT);
    gpio_put(PIN_CS, 1);

    int r = w25q32_init(&flash, &bus, PIN_CS);
    printf("w25q32_init -> %d\n\n", r);
    if (r != EDEV_OK) {
        printf("INIT FAILED — check wiring and power\n");
        while (true) sleep_ms(1000);
    }

    test_jedec_id();
    test_read_write();
    test_power_management();

    printf("\n=== Demo complete ===\n");
    while (true) sleep_ms(1000);
}
