cmake_minimum_required(VERSION 3.13)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT PICO_SDK_PATH)
    if(DEFINED ENV{PICO_SDK_PATH})
        set(PICO_SDK_PATH "$ENV{PICO_SDK_PATH}")
    else()
        set(PICO_SDK_PATH "/opt/pico-sdk")
    endif()
endif()
set(PICO_SDK_PATH "${PICO_SDK_PATH}" CACHE PATH "Path to the Raspberry Pi Pico SDK" FORCE)
if(NOT EXISTS "${PICO_SDK_PATH}/external/pico_sdk_import.cmake")
    message(FATAL_ERROR "Pico SDK import not found under ${PICO_SDK_PATH}")
endif()
include("${PICO_SDK_PATH}/external/pico_sdk_import.cmake")

project(T425_SIM_Canonical C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

set(T425_CORE_ROOT ${CMAKE_CURRENT_LIST_DIR}/../t425_core)
set(T425_RUNTIME_ROOT ${CMAKE_CURRENT_LIST_DIR}/../t425_runtime)
set(T425_PHY_ROOT ${CMAKE_CURRENT_LIST_DIR}/../t425_phy/rp2040)

add_library(t425_core_engine STATIC
    ${T425_CORE_ROOT}/src/t425_core.c
    ${T425_CORE_ROOT}/src/t425_core_system.c
    ${T425_CORE_ROOT}/instruction/t425_instruction.c
    ${T425_CORE_ROOT}/channel/t425_channel.c
    ${T425_CORE_ROOT}/cpu/t425_cpu.c
    ${T425_CORE_ROOT}/memory/t425_memory.c
    ${T425_CORE_ROOT}/scheduler/t425_scheduler.c
    ${T425_CORE_ROOT}/timer/t425_timer.c
    ${T425_CORE_ROOT}/microcode/t425_crom.c
    ${T425_CORE_ROOT}/microcode/t425_datapath.c
    ${T425_CORE_ROOT}/microcode/t425_incrementor.c
    ${T425_CORE_ROOT}/microcode/t425_microentry.c
    ${T425_CORE_ROOT}/microcode/t425_microsequencer.c
    ${T425_CORE_ROOT}/microcode/t425_mir.c
    ${T425_CORE_ROOT}/microcode/t425_mir_fields.c
)
target_include_directories(t425_core_engine PUBLIC
    ${T425_CORE_ROOT}/include
    ${T425_CORE_ROOT}/microcode
    ${T425_CORE_ROOT}/channel
    ${T425_CORE_ROOT}/process
    ${T425_CORE_ROOT}/scheduler
    ${T425_CORE_ROOT}/timer
    ${T425_CORE_ROOT}/phy
)

add_library(t425_core_phy_contract STATIC
    ${T425_CORE_ROOT}/phy/t425_core_phy.c
)
target_include_directories(t425_core_phy_contract PUBLIC
    ${T425_CORE_ROOT}/phy
)

add_library(t425_core_phy_service STATIC
    ${T425_PHY_ROOT}/core_adapter/t425_core_phy_ipc.c
    ${T425_PHY_ROOT}/core_adapter/t425_core_phy_bootstrap.c
)
target_include_directories(t425_core_phy_service PUBLIC
    ${T425_CORE_ROOT}/phy
    ${T425_PHY_ROOT}/core_adapter
)
target_link_libraries(t425_core_phy_service PUBLIC
    t425_core_phy_contract
    pico_util
)

add_library(t425_runtime STATIC
    ${T425_RUNTIME_ROOT}/t425_runtime.c
)
target_include_directories(t425_runtime PUBLIC
    ${T425_RUNTIME_ROOT}
)
target_link_libraries(t425_runtime PUBLIC
    t425_core_phy_service
    t425_core_phy_contract
)

add_library(t425_phy_rp2040 STATIC
    ${T425_PHY_ROOT}/t425_link.c
    ${T425_PHY_ROOT}/core_adapter/t425_core_phy_adapter.c
    ${T425_PHY_ROOT}/core_adapter/t425_core_phy_rp2040.c
    ${T425_PHY_ROOT}/core_adapter/t425_core_phy_ipc.c
)
pico_generate_pio_header(t425_phy_rp2040
    ${T425_PHY_ROOT}/t425_link.pio
)
pico_generate_pio_header(t425_phy_rp2040
    ${T425_PHY_ROOT}/t425_link_rx.pio
)
target_include_directories(t425_phy_rp2040 PUBLIC
    ${T425_PHY_ROOT}
    ${T425_PHY_ROOT}/core_adapter
    ${T425_CORE_ROOT}/phy
)
target_link_libraries(t425_phy_rp2040 PUBLIC
    t425_core_phy_contract
    pico_stdlib
    pico_util
    hardware_clocks
    hardware_dma
    hardware_irq
    hardware_pio
    hardware_gpio
)

add_custom_target(t425_canonical_libraries ALL
    DEPENDS
        t425_core_engine
        t425_core_phy_contract
        t425_core_phy_service
        t425_runtime
        t425_phy_rp2040
)
