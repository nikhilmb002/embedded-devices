#ifndef EDEV_GFX_H
#define EDEV_GFX_H

#include "edev/gfx/canvas.h"

void gfx_set_pixel  (const edev_canvas_t *c, int x, int y, bool on);
void gfx_draw_line  (const edev_canvas_t *c, int x0, int y0, int x1, int y1, bool on);
void gfx_draw_rect  (const edev_canvas_t *c, int x, int y, int w, int h, bool on);
void gfx_fill_rect  (const edev_canvas_t *c, int x, int y, int w, int h, bool on);
void gfx_draw_circle(const edev_canvas_t *c, int cx, int cy, int r, bool on);
void gfx_fill_circle(const edev_canvas_t *c, int cx, int cy, int r, bool on);


/* Text: 5x7 font in a 6x8 cell (1 pixel gap). scale = 1, 2, 3 ... */
void gfx_draw_char  (const edev_canvas_t *c, int x, int y, char ch, int scale, bool on);
void gfx_draw_string(const edev_canvas_t *c, int x, int y, const char *s, int scale, bool on);
int  gfx_text_width (const char *s, int scale);   /* width in pixels */

#endif /* EDEV_GFX_H */