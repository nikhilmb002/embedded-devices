#ifndef EDEV_SSD1306_H
#define EDEV_SSD1306_H

#include <stdint.h>
#include <stdbool.h>
#include "edev/bus/i2c_dev.h"
#include "edev/gfx/canvas.h"

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
void ssd1306_canvas(ssd1306_t *d, edev_canvas_t *c);

int ssd1306_set_contrast(ssd1306_t *d, uint8_t contrast);   /* 0..255 */
int ssd1306_invert(ssd1306_t *d, bool invert);              /* true = inverted colors */
int ssd1306_flip(ssd1306_t *d, bool flip_x, bool flip_y);   /* mirror horizontally / vertically */
int ssd1306_all_pixels_on(ssd1306_t *d, bool on);           /* test mode: light every pixel */

/* Scroll interval = time between steps, in frames. */
typedef enum {
    SSD1306_SCROLL_5_FRAMES   = 0x00,   /* fastest-ish */
    SSD1306_SCROLL_64_FRAMES  = 0x01,
    SSD1306_SCROLL_128_FRAMES = 0x02,
    SSD1306_SCROLL_256_FRAMES = 0x03,   /* slowest */
    SSD1306_SCROLL_3_FRAMES   = 0x04,
    SSD1306_SCROLL_4_FRAMES   = 0x05,
    SSD1306_SCROLL_25_FRAMES  = 0x06,
    SSD1306_SCROLL_2_FRAMES   = 0x07    /* fastest */
} ssd1306_scroll_speed_t;

/* Pages are 8-pixel rows: 0..7. start_page <= end_page. */
int ssd1306_scroll_horizontal(ssd1306_t *d, bool right,
                              uint8_t start_page, uint8_t end_page,
                              ssd1306_scroll_speed_t speed);

/* Horizontal scroll plus a vertical movement of 'v_offset' rows per step (1..63). */
int ssd1306_scroll_diagonal(ssd1306_t *d, bool right,
                            uint8_t start_page, uint8_t end_page,
                            ssd1306_scroll_speed_t speed, uint8_t v_offset);

int ssd1306_scroll_stop(ssd1306_t *d);

#endif /* EDEV_SSD1306_H */