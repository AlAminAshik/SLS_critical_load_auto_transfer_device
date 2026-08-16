cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(16x2-I2C-LCD-rp2040 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(16x2-I2C-LCD-rp2040 main.c lcd_i2c.c)

pico_enable_stdio_usb(16x2-I2C-LCD-rp2040 1)
pico_add_extra_outputs(16x2-I2C-LCD-rp2040)
target_link_libraries(16x2-I2C-LCD-rp2040 pico_stdlib hardware_i2c)