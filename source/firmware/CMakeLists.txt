cmake_minimum_required(VERSION 3.13)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
project(t425_node C CXX ASM)
pico_sdk_init()

add_executable(t425_node
    t425_main.c
    t425_link_driver.c
)
target_include_directories(t425_node PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(t425_node pico_stdlib hardware_pio hardware_uart hardware_clocks)
pico_enable_stdio_usb(t425_node 1)
pico_enable_stdio_uart(t425_node 0)
pico_add_extra_outputs(t425_node)
