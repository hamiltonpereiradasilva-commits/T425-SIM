# Reusable RP2040 CORE<->PHY boundary and Core0/Core1 mailbox component.
add_library(t425_phy_rp2040 STATIC
    t425_link.c
    core_adapter/t425_core_phy_adapter.c
    core_adapter/t425_core_phy_rp2040.c
    core_adapter/t425_core_phy_ipc.c
)

pico_generate_pio_header(t425_phy_rp2040
    ${CMAKE_CURRENT_LIST_DIR}/t425_link.pio
)
pico_generate_pio_header(t425_phy_rp2040
    ${CMAKE_CURRENT_LIST_DIR}/t425_link_rx.pio
)

target_include_directories(t425_phy_rp2040 PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/core_adapter
    ${CMAKE_CURRENT_LIST_DIR}/../../t425_core/phy
)

target_link_libraries(t425_phy_rp2040 PUBLIC
    t425_core_engine
    pico_stdlib
    pico_util
    hardware_clocks
    hardware_dma
    hardware_irq
    hardware_pio
)
