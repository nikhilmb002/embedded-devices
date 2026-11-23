# Wiring: Raspberry Pi Pico 2 + SSD1306 (I2C)

| SSD1306 | Pico 2 |
|---|---|
| VCC | 3V3(OUT), pin 36 |
| GND | GND, pin 38 |
| SDA | GP4, pin 6 |
| SCL | GP5, pin 7 |

Debug UART (optional): FT232RL RX to GP0 (pin 1), TX to GP1 (pin 2),
GND to any Pico GND. Set the FT232RL to 3.3 V. Use 115200 baud.

All devices must share a common ground.