# embedded-devices

Portable embedded device drivers (I2C now; SPI and CAN planned) with
swappable hardware backends. Drivers never include MCU headers, so the
same driver code can run on hardware, bit-banged buses, or a PC mock.

## Features (SSD1306 128x64 OLED over I2C)

- Framebuffer driver: init, clear, set_pixel, flush
- Partial update: dirty-page tracking and `ssd1306_flush_dirty`
- Graphics: lines, rectangles, circles (filled and outline)
- Text: 5x7 font, scaling, newlines, `gfx_printf`
- 1-bit bitmaps and a generated icon set
- Display controls: contrast, invert, flip, all-pixels-on, power save
- Hardware scrolling (horizontal and diagonal), with automatic stop before flush

See `docs/benchmarks.md` for measured flush timings.

## Architecture

```
Application
    |
gfx (shapes, text, bitmaps)  -- draws through a canvas
    |
Chip driver (ssd1306)
    |
i2c_dev (bus + address, register helpers)
    |
bus_i2c (ops table)
    |
Backend (Pico hardware I2C)
```

## Status

| Component | Bus | Status |
|---|---|---|
| SSD1306 128x64 | I2C | experimental (tested on Pico 2) |
| gfx (shapes, text, bitmaps) | none | experimental |
| Pico hardware I2C backend | I2C | experimental |

## Quick start (Pico SDK project)

```powershell
git submodule add https://github.com/nikhilmb002/embedded-devices.git external/embedded-devices
```

In `CMakeLists.txt`, after `pico_sdk_init()`:

```cmake
add_subdirectory(external/embedded-devices)

target_link_libraries(my_app
    pico_stdlib edev_core edev_backend_pico edev_ssd1306 edev_gfx)
```

In your code:

```c
#include "edev/backends/pico/i2c_hw.h"
#include "edev/drivers/display/ssd1306.h"
#include "edev/gfx/gfx.h"

static edev_i2c_bus_t bus;
static ssd1306_t      oled;
static edev_canvas_t  canvas;

edev_pico_i2c_init(&bus, i2c0, 4, 5, 400000);
ssd1306_init(&oled, &bus, 0x3C);
ssd1306_canvas(&oled, &canvas);

gfx_draw_string(&canvas, 0, 0, "Hello", 2, true);
gfx_printf(&canvas, 0, 24, 1, true, "count=%d", 42);
ssd1306_flush(&oled);
```

Notes: `gfx_printf` output is limited to 63 characters. Hardware I2C at
1 MHz is outside the SSD1306 datasheet; 400 kHz is the recommended speed.

## Icons

Icons are drawn as ASCII art in `tools/icons/icons.txt` and converted to
`include/edev/gfx/icons.h` with `tools/make_icons.py`. The generated header is
committed, so users do not need Python.

## Examples and docs

- `examples/ssd1306_demo` - runs every feature in a loop
- `docs/wiring` - pin tables
- `docs/benchmarks.md` - flush timings at 400 kHz and 1 MHz

## Roadmap

Mock backend and host tests, CI, bit-bang I2C, SPI, CAN (MCP2515).

## License

MIT