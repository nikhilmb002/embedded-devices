#include <stdio.h>
#include "pico/stdlib.h"
#include "edev/backends/pico/i2c_hw.h"
#include "edev/drivers/display/ssd1306.h"

static edev_i2c_bus_t bus;
static ssd1306_t      oled;     /* ~1 KB, so keep it static */

int main(void)
{
    stdio_init_all();
    sleep_ms(2000);

    int r = edev_pico_i2c_init(&bus, i2c0, 4, 5, 400 * 1000);
    printf("i2c init -> %d\n", r);

    r = ssd1306_init(&oled, &bus, 0x3C);
    printf("ssd1306_init -> %d\n", r);
    if (r != EDEV_OK) {
        while (true) sleep_ms(1000);
    }

    int x = 0;
    int frame = 0;
    
    while (true) {
        
        ssd1306_clear(&oled);
        
        for (int y = 0; y < SSD1306_HEIGHT; y++) {
            ssd1306_set_pixel(&oled, x,     y, true);
            ssd1306_set_pixel(&oled, x + 1, y, true);
        }

        r = ssd1306_flush(&oled);
        
        if (r != EDEV_OK) printf("flush error %d\n", r);

        if (++frame % 33 == 0) printf("running, x=%d, last flush=%d\n", x, r);

        x = (x + 2) % SSD1306_WIDTH;
        sleep_ms(30);
    }
}