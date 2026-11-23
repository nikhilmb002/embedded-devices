# SSD1306 demo

Cycles through shapes, text, icons, a live counter (partial flush),
contrast/invert and hardware scrolling.

Use it in a Pico SDK project that has this library as a submodule:

1. Copy `main.c` over your project's main file.
2. Link: `pico_stdlib edev_core edev_backend_pico edev_ssd1306 edev_gfx`
3. Wiring: see `docs/wiring/pico2_ssd1306.md` (SDA = GP4, SCL = GP5, address 0x3C).