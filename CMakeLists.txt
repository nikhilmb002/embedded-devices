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
)
target_include_directories(edev_core PUBLIC include)