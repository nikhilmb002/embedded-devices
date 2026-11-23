#ifndef EDEV_CANVAS_H
#define EDEV_CANVAS_H

#include <stdbool.h>

/* A drawable surface. Any display driver can provide one. */
typedef struct {
    
    void *ctx;                                         /* the driver's own struct */
    int   width;
    int   height;
    void (*set_pixel)(void *ctx, int x, int y, bool on);
} edev_canvas_t;

#endif /* EDEV_CANVAS_H */