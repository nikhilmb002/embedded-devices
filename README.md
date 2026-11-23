# embedded-devices

Portable embedded device drivers 
swappable hardware backends. Drivers never include MCU headers, so the
same driver code runs on hardware, bit-banged buses, or a PC mock.

## Architecture

```
Chip drivers      ssd1306, ...
      |
Bus interfaces    bus_i2c (ops table)
      |
Backends          Pico hardware I2C
```

## Status

| Driver / backend | Bus | Status |
|---|---|---|
| SSD1306 128x64 OLED | I2C | experimental (tested on Pico 2) |
| Pico hardware I2C backend | I2C | experimental |

## Quick start (Pico SDK project)

```powershell
git submodule add https://github.com/<your-username>/embedded-devices.git external/embedded-devices
```

In your `CMakeLists.txt`, after `pico_sdk_init()`:

```cmake
add_subdirectory(external/embedded-devices)

target_link_libraries(my_app pico_stdlib edev_core edev_backend_pico edev_ssd1306)
```

In your code:

```c
#include "edev/backends/pico/i2c_hw.h"
#include "edev/drivers/display/ssd1306.h"

static edev_i2c_bus_t bus;
static ssd1306_t oled;

edev_pico_i2c_init(&bus, i2c0, 4, 5, 400000);
ssd1306_init(&oled, &bus, 0x3C);
ssd1306_set_pixel(&oled, 10, 10, true);
ssd1306_flush(&oled);
```

See `examples/ssd1306_demo` and `docs/wiring`.

## Roadmap

Mock backend and host tests, SPI, bit-bang I2C, CAN (MCP2515).

## License

MIT