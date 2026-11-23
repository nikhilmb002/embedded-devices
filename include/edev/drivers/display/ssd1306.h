#ifndef EDEV_SSD1306_H
#define EDEV_SSD1306_H

#include <stdint.h>
#include <stdbool.h>
#include "edev/bus/i2c_dev.h"

#define SSD1306_WIDTH   128
#define SSD1306_HEIGHT  64
#define SSD1306_FB_SIZE (SSD1306_WIDTH * SSD1306_HEIGHT / 8)   /* 1024 bytes */

/*
 * The caller owns this struct (declare it static or global, not on a small
 * stack: it is about 1 KB). No hidden globals, so you can have several displays.
 * buf[0] is always the 0x40 "data" control byte, buf[1..] is the framebuffer.
 */
typedef struct {
    edev_i2c_dev_t dev;
    uint8_t        buf[1 + SSD1306_FB_SIZE];
} ssd1306_t;

int  ssd1306_init(ssd1306_t *d, edev_i2c_bus_t *bus, uint8_t addr);
void ssd1306_clear(ssd1306_t *d);
void ssd1306_set_pixel(ssd1306_t *d, int x, int y, bool on);
int  ssd1306_flush(ssd1306_t *d);              /* send framebuffer to the screen */
int  ssd1306_display_on(ssd1306_t *d, bool on);

#endif /* EDEV_SSD1306_H */