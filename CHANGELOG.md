# Changelog

## v0.2.0
- gfx layer: canvas interface, lines, rectangles, circles
- 5x7 font, `gfx_draw_string`, `gfx_text_width`, `gfx_printf`
- 1-bit bitmap drawing and a generated icon set (`tools/make_icons.py`)
- SSD1306: contrast, invert, flip, all-pixels-on controls
- SSD1306: hardware horizontal and diagonal scrolling
- SSD1306: dirty-page tracking, `ssd1306_flush_dirty`, automatic scroll stop
- Benchmarks for flush timing
- Font table moved to `font5x7.c`; `edev_gfx` links `edev_core`

## v0.1.0
- Error codes and I2C bus interface (ops table)
- i2c_dev helper (bus + address, register read/write)
- Pico hardware I2C backend
- SSD1306 128x64 driver (init, clear, set_pixel, flush)
- SSD1306 demo example