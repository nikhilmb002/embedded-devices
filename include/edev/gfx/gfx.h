#ifndef EDEV_GFX_H
#define EDEV_GFX_H

#include "edev/gfx/canvas.h"

void gfx_set_pixel  (const edev_canvas_t *c, int x, int y, bool on);
void gfx_draw_line  (const edev_canvas_t *c, int x0, int y0, int x1, int y1, bool on);
void gfx_draw_rect  (const edev_canvas_t *c, int x, int y, int w, int h, bool on);
void gfx_fill_rect  (const edev_canvas_t *c, int x, int y, int w, int h, bool on);
void gfx_draw_circle(const edev_canvas_t *c, int cx, int cy, int r, bool on);
void gfx_fill_circle(const edev_canvas_t *c, int cx, int cy, int r, bool on);

#endif /* EDEV_GFX_H */