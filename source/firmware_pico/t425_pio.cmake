# Include this fragment from the firmware target CMakeLists.txt.
# pico_generate_pio_header(TARGET FILE) invokes pioasm and adds generated headers.
pico_generate_pio_header(t425_firmware ${CMAKE_CURRENT_LIST_DIR}/t425_pio_tx.pio)
pico_generate_pio_header(t425_firmware ${CMAKE_CURRENT_LIST_DIR}/t425_pio_rx.pio)
