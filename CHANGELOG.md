# Changelog

## Unreleased
- Add portable MPU6050 driver with raw/scaled readings, configuration, FIFO,
  interrupts, auxiliary I2C controls, self-test/offset access, and DMP image upload.
- Accept MPU6500 devices (WHO_AM_I `0x70`) for the shared motion-sample path.
- Add a host mock-bus test for sensor decoding, scaling, errors, and DMP memory.

## v0.1.0
- Error codes and I2C bus interface (ops table)
- i2c_dev helper (bus + address, register read/write)
- Pico hardware I2C backend
- SSD1306 128x64 driver (init, clear, set_pixel, flush)
- SSD1306 demo example
