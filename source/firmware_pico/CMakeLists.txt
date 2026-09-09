cmake_minimum_required(VERSION 3.13)
if(NOT DEFINED ENV{PICO_SDK_PATH})
    message(FATAL_ERROR "PICO_SDK_PATH is not set")
endif()

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include(pico_sdk_import.cmake)
project(t425_pico_v15 C CXX ASM)

if (NOT DEFINED PICO_BOARD)
    set(PICO_BOARD pico)
endif()

if (PICO_BOARD STREQUAL "pico")
    set(T425_PHYSICAL_FLASH_SIZE 2097152 CACHE INTERNAL "RP2040 Pico Flash")
else()
    message(FATAL_ERROR "T425 v2.84 currently supports only PICO_BOARD=pico")
endif()

if (T425_CONFIG_REGION_SIZE GREATER_EQUAL T425_PHYSICAL_FLASH_SIZE)
    message(FATAL_ERROR "T425 config region exceeds physical Flash")
endif()

pico_sdk_init()

set(T425_LINK_BAUD 115200 CACHE STRING "T425 physical link baud")
set(T425_CONFIG_REGION_SIZE 8192 CACHE INTERNAL "T425 persistent Flash region")
set(T425_PHYSICAL_FLASH_SIZE 2097152 CACHE INTERNAL "RP2040 Pico Flash size")

add_executable(t425_pico_v15
    main.c
    t425_pio_hw.c
    t425_crc16.c
    t425_link_test.c
    ../src/t425_crc16.c
    ../src/t425_frame.c
    ../src/t425_stream.c
    ../src/t425_ack.c
    ../src/t425_node_config.c
    t425_interfaces.c
    ../src/t425_runtime.c
    ../src/t425_scheduler.c
    ../src/t425_vm_scheduler.c
    ../src/t425_supervisor.c

    t425_config_store.c
    t425_flash_slots.c
    t425_flash_transaction.c
    t425_flash_hw.c
    t425_flash_rp2040_real.c
    t425_flash_layout_check.c
    t425_static_contracts.c
    t425_config_canonical.c
    t425_config_staging.c
    t425_usb_config_session.c
    t425_usb_config_commit.c
    t425_usb_config_protocol.c
)

pico_generate_pio_header(t425_pico_v15
    ${CMAKE_CURRENT_LIST_DIR}/t425_uart8_8n1.pio)

target_compile_definitions(t425_pico_v15 PRIVATE
    T425_LINK_BAUD=${T425_LINK_BAUD}
    T425_CONFIG_REGION_SIZE=${T425_CONFIG_REGION_SIZE}
    T425_PHYSICAL_FLASH_SIZE=${T425_PHYSICAL_FLASH_SIZE})

target_include_directories(t425_pico_v15 PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../include)

target_link_libraries(t425_pico_v15 PRIVATE
    pico_stdlib
    hardware_pio
    hardware_flash
    pico_flash)

# Reserve the final 8 KiB of the physical 2 MiB Flash for persistent A/B config.
math(EXPR T425_FIRMWARE_FLASH_SIZE
     "${T425_PHYSICAL_FLASH_SIZE} - ${T425_CONFIG_REGION_SIZE}")

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/t425_pico_v15)

pico_override_flash_size(t425_pico_v15 ${T425_FIRMWARE_FLASH_SIZE})

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/t425_linker)

file(WRITE
    ${CMAKE_CURRENT_BINARY_DIR}/t425_linker/section_extra_post_end.incl
    "__t425_config_start__ = ORIGIN(FLASH) + LENGTH(FLASH);\n"
    "__t425_config_end__ = __t425_config_start__ + ${T425_CONFIG_REGION_SIZE};\n"
    "ASSERT((__t425_config_start__ % 4096) == 0, \"T425 config start not sector aligned\")\n"
    "ASSERT((__t425_config_end__ % 4096) == 0, \"T425 config end not sector aligned\")\n"
    "ASSERT((__t425_config_end__ - __t425_config_start__) == ${T425_CONFIG_REGION_SIZE}, \"T425 config size mismatch\")\n")

pico_add_linker_script_override_path(
    t425_pico_v15
    ${CMAKE_CURRENT_BINARY_DIR}/t425_linker
    FILES section_extra_post_end.incl
)

pico_enable_stdio_usb(t425_pico_v15 1)
pico_enable_stdio_uart(t425_pico_v15 0)
pico_add_extra_outputs(t425_pico_v15)
