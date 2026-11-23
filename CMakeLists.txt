cmake_minimum_required(VERSION 3.13)

# Only declare the project when built standalone (e.g. host tests).
# When added as a subdirectory of a Pico project, the parent owns project().
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(embedded_devices C)
endif()

option(EDEV_BUILD_EXAMPLES "Build examples" OFF)
option(EDEV_BUILD_TESTS    "Build host unit tests" OFF)

# ---- Portable core: no hardware dependencies ----
add_library(edev_core STATIC
    src/bus/i2c_dev.c
    src/bus/spi_dev.c
)
target_include_directories(edev_core PUBLIC include)

# ---- Pico hardware backend: only when the Pico SDK is present ----
if(COMMAND pico_sdk_init OR TARGET pico_stdlib)
    add_library(edev_backend_pico STATIC
        src/backends/pico/i2c_hw.c
        src/backends/pico/spi_hw.c
    )
    target_include_directories(edev_backend_pico PUBLIC include)
    target_link_libraries(edev_backend_pico PUBLIC
        edev_core
        pico_stdlib
        hardware_i2c
        hardware_spi
    )
endif()

# ---- Chip drivers: portable, depend only on edev_core ----
add_library(edev_ssd1306 STATIC
    src/drivers/display/ssd1306.c
)
target_include_directories(edev_ssd1306 PUBLIC include)
target_link_libraries(edev_ssd1306 PUBLIC edev_core)

# ---- W25Q32 SPI flash driver ----
add_library(edev_w25q32 STATIC
    src/drivers/flash/w25q32.c
)
target_include_directories(edev_w25q32 PUBLIC include)
target_link_libraries(edev_w25q32 PUBLIC edev_core)

# ---- Graphics layer: portable, works with any canvas ----
add_library(edev_gfx STATIC
    src/gfx/gfx.c
    src/gfx/font5x7.c
)
target_include_directories(edev_gfx PUBLIC include)
target_link_libraries(edev_gfx PUBLIC edev_core)