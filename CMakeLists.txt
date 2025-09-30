cmake_minimum_required(VERSION 3.13)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(fra_rp2350 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the Pico SDK
pico_sdk_init()

# Add executable
add_executable(fra_rp2350
    src/main.c
    src/adc_dma.c
    src/ad9833.c
    src/goertzel.c
    src/mqtt_client.c
    src/sweep.c
)

# Pull in common dependencies
target_link_libraries(fra_rp2350
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    hardware_adc
    hardware_dma
    hardware_spi
    hardware_timer
    pico_lwip_mqtt
)

# Enable USB serial output, disable UART
pico_enable_stdio_usb(fra_rp2350 1)
pico_enable_stdio_uart(fra_rp2350 0)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(fra_rp2350)

# Optimization flags for production
# Uncomment for release build:
# target_compile_options(fra_rp2350 PRIVATE -O3 -march=armv8-m.main+fp -mfpu=fpv5-sp-d16 -mfloat-abi=hard)

# Debug flags (default)
target_compile_options(fra_rp2350 PRIVATE -O0 -g)

# Warning flags
target_compile_options(fra_rp2350 PRIVATE -Wall -Wextra)
