cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(GPSLogger VERSION 1.0)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# initialize the SD library
add_subdirectory(extern/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src)

# initialize our project source
add_subdirectory(src)
