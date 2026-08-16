cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(mainCLAT C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(mainCLAT mainCLAT.c lcd_i2c.c bat_calc.c)

pico_enable_stdio_usb(mainCLAT 1)   # enable printf over USB serial
pico_enable_stdio_uart(mainCLAT 0)  # disable UART stdio (optional, avoids duplicate output)

pico_add_extra_outputs(mainCLAT)
target_link_libraries(mainCLAT pico_stdlib hardware_adc hardware_i2c)

# Enable float formatting support in printf/snprintf (newlib-nano strips it by default)
pico_add_link_depend(mainCLAT ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt)
target_link_options(mainCLAT PRIVATE -u _printf_float)