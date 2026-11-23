# W25Q32 Flash Demo

Exercises every W25Q32 driver function and prints results to UART.

## Wiring

See [docs/wiring/pico2_w25q32.md](../../docs/wiring/pico2_w25q32.md).

Default pins: MISO = GP16, CS = GP17, SCK = GP18, MOSI = GP19.
Tie /WP and /HOLD to 3.3V.

## Build

In a Pico SDK project that includes `embedded-devices` as a subdirectory:

```cmake
add_subdirectory(external/embedded-devices)

add_executable(w25q32_demo examples/w25q32_demo/main.c)
target_link_libraries(w25q32_demo
    pico_stdlib edev_core edev_backend_pico edev_w25q32)
pico_add_extra_outputs(w25q32_demo)
pico_enable_stdio_usb(w25q32_demo 1)
```

## Expected output

```
=== W25Q32 Flash Demo ===

w25q32_init -> 0

JEDEC ID: mfr=0xEF type=0x40 cap=0x16 (r=0)
Erasing sector 0... r=0
Sector erased: YES (r=0)
Writing 64 bytes... r=0
Readback match: YES (r=0)
Cross-page write test (addr=0xF0, 32 bytes)... r=0
Cross-page readback: YES (r=0)
Power down... r=0
Power up... r=0
After wakeup: mfr=0xEF (r=0)

=== Demo complete ===
```
