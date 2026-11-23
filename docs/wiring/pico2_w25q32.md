# Pico 2 ↔ W25Q32 Wiring

| W25Q32 Pin | Pico 2 Pin | GPIO | Notes |
|---|---|---|---|
| CLK | Pin 24 | GP18 | SPI0 SCK |
| DI (MOSI) | Pin 25 | GP19 | SPI0 TX |
| DO (MISO) | Pin 21 | GP16 | SPI0 RX |
| CS | Pin 22 | GP17 | Software-controlled GPIO |
| VCC | 3V3 (Pin 36) | — | 3.3 V supply |
| GND | GND (Pin 38) | — | Ground |
| /WP | 3V3 | — | Tie high for normal operation |
| /HOLD | 3V3 | — | Tie high for normal operation |

> **Note:** /WP (Write Protect) and /HOLD must be tied to VCC.
> Leaving them floating may cause erratic behaviour.
