#include "edev/gfx/gfx.h"
#include "edev/gfx/font5x7.h"
#include <stdlib.h>

void gfx_set_pixel(const edev_canvas_t *c, int x, int y, bool on) {

    if (!c || !c->set_pixel) return;
    if (x < 0 || x >= c->width || y < 0 || y >= c->height) return;   /* clip */
    
    c->set_pixel(c->ctx, x, y, on);
}

/* Bresenham line algorithm: integer math only. */
void gfx_draw_line(const edev_canvas_t *c, int x0, int y0, int x1, int y1, bool on) {

    int dx =  abs(x1 - x0), sx = (x0 < x1) ? 1 : -1;
    int dy = -abs(y1 - y0), sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;

    for (;;) {

        gfx_set_pixel(c, x0, y0, on);

        if (x0 == x1 && y0 == y1) break;
        
        int e2 = 2 * err;
 
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void gfx_draw_rect(const edev_canvas_t *c, int x, int y, int w, int h, bool on) {

    if (w <= 0 || h <= 0) return;

    gfx_draw_line(c, x,         y,         x + w - 1, y,         on);
    gfx_draw_line(c, x,         y + h - 1, x + w - 1, y + h - 1, on);
    gfx_draw_line(c, x,         y,         x,         y + h - 1, on);
    gfx_draw_line(c, x + w - 1, y,         x + w - 1, y + h - 1, on);
}

void gfx_fill_rect(const edev_canvas_t *c, int x, int y, int w, int h, bool on) {

    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
            gfx_set_pixel(c, x + i, y + j, on);
}

/* Midpoint circle algorithm: draws 8 symmetric points per step. */
void gfx_draw_circle(const edev_canvas_t *c, int cx, int cy, int r, bool on) {

    if (r < 0) return;

    int x = r, y = 0, err = 1 - r;

    while (x >= y) {

        gfx_set_pixel(c, cx + x, cy + y, on);
        gfx_set_pixel(c, cx - x, cy + y, on);
        gfx_set_pixel(c, cx + x, cy - y, on);
        gfx_set_pixel(c, cx - x, cy - y, on);
        gfx_set_pixel(c, cx + y, cy + x, on);
        gfx_set_pixel(c, cx - y, cy + x, on);
        gfx_set_pixel(c, cx + y, cy - x, on);
        gfx_set_pixel(c, cx - y, cy - x, on);
        y++;
        if (err < 0) err += 2 * y + 1;
        else { x--; err += 2 * (y - x) + 1; }
    }
}

void gfx_fill_circle(const edev_canvas_t *c, int cx, int cy, int r, bool on) {

    if (r < 0) return;
    for (int j = -r; j <= r; j++)
        for (int i = -r; i <= r; i++)
            if (i * i + j * j <= r * r)
                gfx_set_pixel(c, cx + i, cy + j, on);
}

void gfx_draw_char(const edev_canvas_t *c, int x, int y, char ch, int scale, bool on)
{
    if (scale < 1) scale = 1;
    if (ch < FONT5X7_FIRST || ch > FONT5X7_LAST) ch = '?';
    const uint8_t *glyph = font5x7[ch - FONT5X7_FIRST];

    for (int col = 0; col < FONT5X7_WIDTH; col++) {
        for (int row = 0; row < FONT5X7_HEIGHT; row++) {
            if (glyph[col] & (1u << row)) {
                gfx_fill_rect(c, x + col * scale, y + row * scale, scale, scale, on);
            }
        }
    }
}

void gfx_draw_string(const edev_canvas_t *c, int x, int y, const char *s, int scale, bool on) {

    if (!s) return;
    if (scale < 1) scale = 1;
    
    int cx = x;
    
    while (*s) {
    
        if (*s == '\n') {                       /* new line */
            cx = x;
            y += (FONT5X7_HEIGHT + 1) * scale;
        } else {
            gfx_draw_char(c, cx, y, *s, scale, on);
            cx += (FONT5X7_WIDTH + 1) * scale;  /* 5 pixels + 1 gap */
        }
        s++;
    }
}

int gfx_text_width(const char *s, int scale) {

    if (!s) return 0;
    if (scale < 1) scale = 1;
    
    int n = 0;
    
    while (s[n] && s[n] != '\n') n++;
    
    return n > 0 ? n * (FONT5X7_WIDTH + 1) * scale - scale : 0;
}