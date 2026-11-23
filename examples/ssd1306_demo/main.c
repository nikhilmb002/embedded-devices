/* SSD1306 demo: shapes, text, printf, icons, controls, scroll, partial flush. */
#include <stdio.h>
#include "pico/stdlib.h"
#include "edev/backends/pico/i2c_hw.h"
#include "edev/drivers/display/ssd1306.h"
#include "edev/gfx/gfx.h"
#include "edev/gfx/icons.h"

static edev_i2c_bus_t bus;
static ssd1306_t      oled;
static edev_canvas_t  cv;

static void begin(const char *name) {
    
    printf("demo: %s\n", name);
    ssd1306_clear(&oled);
}

static void demo_shapes(void) {

    begin("shapes");
    gfx_draw_rect(&cv, 0, 0, 128, 64, true);
    gfx_draw_line(&cv, 0, 0, 127, 63, true);
    gfx_draw_line(&cv, 0, 63, 127, 0, true);
    gfx_draw_circle(&cv, 32, 32, 20, true);
    gfx_fill_circle(&cv, 96, 32, 14, true);
    ssd1306_flush(&oled);
    sleep_ms(3000);
}

static void demo_text(void) {

    begin("text");
    gfx_draw_string(&cv, 0, 0, "Hello Pico 2!", 1, true);
    gfx_draw_string(&cv, 0, 12, "embedded-devices", 1, true);
    gfx_draw_string(&cv, 0, 28, "BIG", 3, true);
    ssd1306_flush(&oled);
    sleep_ms(3000);
}

static void demo_icons(void) {

    begin("icons");
    const uint8_t *const set[] = {
        icon_heart, icon_check, icon_cross, icon_smiley,
        icon_star, icon_home, icon_lock, icon_battery
    };
    
    for (int i = 0; i < 8; i++)
        gfx_draw_bitmap(&cv, i * 16, 4, set[i], 16, 16, true);
    
        gfx_draw_string(&cv, 0, 36, "25.5", 2, true);
    gfx_draw_bitmap(&cv, 52, 36, icon_degree, ICON_DEGREE_W, ICON_DEGREE_H, true);
    gfx_draw_string(&cv, 60, 36, "C", 2, true);
    ssd1306_flush(&oled);
    sleep_ms(3000);
}

static void demo_counter(void) {

    begin("counter (partial flush)");
    gfx_draw_string(&cv, 0, 0, "Counter", 1, true);
    ssd1306_flush(&oled);
    
    for (unsigned n = 0; n < 30; n++) {
    
        gfx_fill_rect(&cv, 0, 24, 128, 16, false);
        gfx_printf(&cv, 0, 24, 2, true, "%u", n);
        ssd1306_flush_dirty(&oled);          /* sends only the changed pages */
        sleep_ms(100);
    }
}

static void demo_controls(void) {

    begin("controls");
    gfx_draw_string(&cv, 0, 20, "Contrast", 2, true);
    ssd1306_flush(&oled);
   
    for (int v = 255; v >= 0; v -= 5) { 
        ssd1306_set_contrast(&oled, (uint8_t)v); sleep_ms(15); 
    }
    
    for (int v = 0; v <= 255; v += 5)  { 
        
        ssd1306_set_contrast(&oled, (uint8_t)v); sleep_ms(15); 
    }
   
    ssd1306_invert(&oled, true);  sleep_ms(1000);
    ssd1306_invert(&oled, false); sleep_ms(500);
}

static void demo_scroll(void) {

    begin("scroll");
    gfx_draw_string(&cv, 0, 24, "SCROLL", 3, true);
    ssd1306_flush(&oled);
    ssd1306_scroll_horizontal(&oled, true, 0, 7, SSD1306_SCROLL_2_FRAMES);
    sleep_ms(3000);
    ssd1306_scroll_horizontal(&oled, false, 0, 7, SSD1306_SCROLL_2_FRAMES);
    sleep_ms(3000);
    ssd1306_scroll_stop(&oled);
    ssd1306_flush(&oled);                    /* redraw cleanly after scrolling */
}

int main(void) {

    stdio_init_all();
    sleep_ms(2000);

    edev_pico_i2c_init(&bus, i2c0, 4, 5, 400 * 1000);
    int r = ssd1306_init(&oled, &bus, 0x3C);
    printf("ssd1306_init -> %d\n", r);
   
    if (r != EDEV_OK) while (true) sleep_ms(1000);
   
    ssd1306_canvas(&oled, &cv);

    while (true) {

        demo_shapes();
        demo_text();
        demo_icons();
        demo_counter();
        demo_controls();
        demo_scroll();
    }
}