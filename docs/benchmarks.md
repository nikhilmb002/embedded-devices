# Benchmarks: SSD1306 flush timing

Measured on real hardware with `time_us_64()` around each call.

## Setup

| Item | Value |
|---|---|
| MCU | Raspberry Pi Pico 2 (RP2350) |
| Display | SSD1306 128x64 OLED, I2C address 0x3C |
| I2C | i2c0, SDA = GP4, SCL = GP5 |
| Pull-ups | On the display module, plus the Pico's weak internal pull-ups |
| Wiring | Short jumper wires |
| Library version | v0.2.0 development (dirty-page tracking) |
| Build | Release, Pico SDK 2.3.1 |

## Results

| Operation | 400 kHz | 1 MHz |
|---|---|---|
| Full flush (1025 bytes) | 26076 us | 11541 us |
| Single-page flush (1 page of 8) | 3512 us | 1549 us |
| `flush_dirty` with nothing changed | 1 us | 1 us |

Scroll safety: after `ssd1306_flush_dirty` was called during an active hardware
scroll, `oled.scrolling` was 0 (the scroll was stopped automatically) and the
picture was redrawn cleanly.

## Analysis

- **Cost per byte:** about 25 us per byte at 400 kHz (9 bit-times per byte,
  including the ACK bit), about 11 us at 1 MHz.
- **Full flush speedup at 1 MHz:** 26076 / 11541 = about 2.3x. It is less than
  the 2.5x clock ratio because each transaction has fixed overhead (START,
  address byte, STOP) and the window setup commands.
- **Partial flush benefit:** one page sends 129 data bytes plus window commands,
  about 7.4x less time than a full flush (26076 / 3512 at 400 kHz).
- **Nothing changed:** the dirty check returns immediately, with no I2C traffic.
- **Typical use:** redrawing one line of text costs 1.5 ms at 1 MHz instead of
  26 ms with a full flush at 400 kHz, which is about 17x less.

## Notes

- 1 MHz is outside the SSD1306 datasheet specification (400 kHz). It worked on
  this module with short wires. Other modules or longer wires may not work, so
  400 kHz is the recommended default.
- Numbers vary slightly between runs (a few tens of microseconds).

## How to reproduce

Build `examples/ssd1306_demo` with the speed-test code (the `run_test()` function
that calls `ssd1306_flush`, `ssd1306_flush_dirty` and `time_us_64()`), and read the
results from the UART console at 115200 baud.