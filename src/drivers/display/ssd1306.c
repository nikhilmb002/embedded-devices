#include "edev/drivers/display/ssd1306.h"
#include <string.h>

#define SSD1306_CTRL_CMD   0x00   /* following bytes are commands */
#define SSD1306_CTRL_DATA  0x40   /* following bytes are display data */

#define FB(d) ((d)->buf + 1)

/* Control byte 0x00 + N command bytes, sent as ONE I2C transaction. */
static int send_cmds(ssd1306_t *d, const uint8_t *cmds, size_t n) {
   
    uint8_t tmp[32];
    if (n == 0 || n > sizeof(tmp) - 1) return EDEV_ERR_INVAL;
    tmp[0] = SSD1306_CTRL_CMD;
    memcpy(&tmp[1], cmds, n);
    return edev_i2c_dev_write(&d->dev, tmp, n + 1);
}

int ssd1306_init(ssd1306_t *d, edev_i2c_bus_t *bus, uint8_t addr) {
 
    if (!d || !bus) return EDEV_ERR_INVAL;

    static const uint8_t seq[] = {
        0xAE,         /* display off */
        0xD5, 0x80,   /* clock divide / oscillator */
        0xA8, 0x3F,   /* multiplex ratio = 64 */
        0xD3, 0x00,   /* display offset = 0 */
        0x40,         /* start line = 0 */
        0x8D, 0x14,   /* charge pump on */
        0x20, 0x00,   /* horizontal addressing mode */
        0xA1,         /* segment remap (flip X) */
        0xC8,         /* COM scan direction (flip Y) */
        0xDA, 0x12,   /* COM pins config */
        0x81, 0xCF,   /* contrast */
        0xD9, 0xF1,   /* pre-charge */
        0xDB, 0x40,   /* VCOMH level */
        0xA4,         /* show RAM contents */
        0xA6,         /* normal (not inverted) */
        0xAF          /* display on */
    };

    edev_i2c_dev_init(&d->dev, bus, addr);
    d->buf[0] = SSD1306_CTRL_DATA;
    ssd1306_clear(d);
    return send_cmds(d, seq, sizeof(seq));
}

void ssd1306_clear(ssd1306_t *d) {
   
    if (!d) return;
    memset(FB(d), 0, SSD1306_FB_SIZE);
}

void ssd1306_set_pixel(ssd1306_t *d, int x, int y, bool on) {

    if (!d || x < 0 || x >= SSD1306_WIDTH || y < 0 || y >= SSD1306_HEIGHT) return;
    uint8_t *p = &FB(d)[x + (y / 8) * SSD1306_WIDTH];
    if (on) *p |=  (uint8_t)(1u << (y % 8));
    else    *p &= (uint8_t)~(1u << (y % 8));
}

int ssd1306_flush(ssd1306_t *d) {

    if (!d) return EDEV_ERR_INVAL;

    static const uint8_t window[] = {
        0x21, 0, SSD1306_WIDTH - 1,   /* column range */
        0x22, 0, 7                    /* page range */
    };
    int r = send_cmds(d, window, sizeof(window));
    if (r != EDEV_OK) return r;

    return edev_i2c_dev_write(&d->dev, d->buf, sizeof(d->buf));
}

int ssd1306_display_on(ssd1306_t *d, bool on) {

    if (!d) return EDEV_ERR_INVAL;
    uint8_t c = on ? 0xAF : 0xAE;
    return send_cmds(d, &c, 1);
}

static void canvas_set_pixel(void *ctx, int x, int y, bool on) {

    ssd1306_set_pixel((ssd1306_t *)ctx, x, y, on);
}

void ssd1306_canvas(ssd1306_t *d, edev_canvas_t *c) {

    if (!d || !c) return;
    
    c->ctx       = d;
    c->width     = SSD1306_WIDTH;
    c->height    = SSD1306_HEIGHT;
    c->set_pixel = canvas_set_pixel;
}