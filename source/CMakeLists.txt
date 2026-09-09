cmake_minimum_required(VERSION 3.13)

# Pico SDK is expected through PICO_SDK_PATH.
include(pico_sdk_import.cmake)

project(T425_SIM_Pico_Nodes C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# One common firmware, four target executables.
set(PICO_NODES 0 1 2 3)

foreach(NODE_ID IN LISTS PICO_NODES)
    add_executable(t425_pico${NODE_ID}
        src/main.c
        src/t425_vm.c
        src/t425_scheduler.c
        src/t425_channels.c
        src/t425_debug.c
        src/t425_protocol.c
        src/t425_links.c
        src/t425_selftest.c
        src/t425_runtime.c
        src/t425_process.c
        src/t425_timer.c
        src/t425_occam.c
        src/t425_alt.c
        src/t425_occam_parser.c
        src/t425_node_sim.c
        src/t425_occam_vm.c
    )

    target_compile_definitions(t425_pico${NODE_ID} PRIVATE
        T425_NODE_ID=${NODE_ID}
    )

    pico_generate_pio_header(t425_pico${NODE_ID}
        ${CMAKE_CURRENT_LIST_DIR}/pio/link_uart.pio
    )

    target_include_directories(t425_pico${NODE_ID} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/include
    )

    target_link_libraries(t425_pico${NODE_ID}
        pico_stdlib
        hardware_pio
        hardware_uart
        hardware_timer
        hardware_sync
    )

    pico_enable_stdio_usb(t425_pico${NODE_ID} 1)
    pico_enable_stdio_uart(t425_pico${NODE_ID} 0)

    pico_add_extra_outputs(t425_pico${NODE_ID})
endforeach()


# Optional host-side runtime test (requires a native C compiler).
add_executable(test_runtime tests/test_runtime.c src/t425_runtime.c
        src/t425_process.c
        src/t425_timer.c
        src/t425_occam.c
        src/t425_alt.c
        src/t425_occam_parser.c
        src/t425_node_sim.c
        src/t425_occam_vm.c
    src/t425_channels.c src/t425_scheduler.c src/t425_vm.c)
target_include_directories(test_runtime PRIVATE include)

add_executable(test_process_timer tests/test_process_timer.c
    src/t425_process.c src/t425_timer.c
        src/t425_occam.c
        src/t425_alt.c
        src/t425_occam_parser.c
        src/t425_node_sim.c
        src/t425_occam_vm.c)
target_include_directories(test_process_timer PRIVATE include)

add_executable(test_occam_alt tests/test_occam_alt.c
    src/t425_occam.c src/t425_alt.c
        src/t425_occam_parser.c
        src/t425_node_sim.c
        src/t425_occam_vm.c src/t425_channels.c src/t425_timer.c)
target_include_directories(test_occam_alt PRIVATE include)

add_executable(test_occam_vm tests/test_occam_vm.c src/t425_occam_vm.c src/t425_node_sim.c
    src/t425_channels.c src/t425_timer.c)
target_include_directories(test_occam_vm PRIVATE include)
add_executable(test_parser tests/test_parser.c src/t425_occam_parser.c)
target_include_directories(test_parser PRIVATE include)

add_executable(test_link_packet tests/test_link_packet.c src/t425_link_packet.c)
target_include_directories(test_link_packet PRIVATE include)
add_executable(test_network tests/test_network.c src/t425_network.c)
target_include_directories(test_network PRIVATE include)

add_executable(test_link_stream tests/test_link_stream.c src/t425_link_packet.c src/t425_link_stream.c)
target_include_directories(test_link_stream PRIVATE include)
add_executable(test_link_node tests/test_link_node.c src/t425_link_packet.c src/t425_link_stream.c src/t425_link_node.c)
target_include_directories(test_link_node PRIVATE include)

add_executable(test_pio_transport tests/test_pio_transport.c src/t425_pio_transport.c)
target_include_directories(test_pio_transport PRIVATE include)

add_executable(test_v13_config tests/test_v13_config.c)
target_include_directories(test_v13_config PRIVATE include)

add_executable(test_link_control tests/test_link_control.c src/t425_link_packet.c src/t425_link_control.c)
target_include_directories(test_link_control PRIVATE include)

add_executable(test_reliable_link tests/test_reliable_link.c src/t425_link_packet.c src/t425_reliable_link.c)
target_include_directories(test_reliable_link PRIVATE include)

add_executable(test_occam_chan tests/test_occam_chan.c src/t425_link_packet.c src/t425_occam_chan.c)
target_include_directories(test_occam_chan PRIVATE include)

add_executable(test_occam_runtime tests/test_occam_runtime.c src/t425_occam_runtime.c)
target_include_directories(test_occam_runtime PRIVATE include)

add_executable(test_rendezvous tests/test_rendezvous.c src/t425_rendezvous.c)
target_include_directories(test_rendezvous PRIVATE include)

add_executable(test_remote_rdv tests/test_remote_rdv.c src/t425_remote_rdv.c)
target_include_directories(test_remote_rdv PRIVATE include)

add_executable(test_dist_runtime tests/test_dist_runtime.c src/t425_dist_runtime.c src/t425_remote_rdv.c)
target_include_directories(test_dist_runtime PRIVATE include)

add_executable(test_router tests/test_router.c src/t425_router.c src/t425_remote_rdv.c)
target_include_directories(test_router PRIVATE include)

add_executable(test_hardware_profile tests/test_hardware_profile.c)

add_executable(test_crc16 tests/test_crc16.c src/t425_crc16.c)
target_include_directories(test_crc16 PRIVATE include)

add_executable(test_byte_stream tests/test_byte_stream.c)

add_executable(test_frame tests/test_frame.c src/t425_frame.c)
target_include_directories(test_frame PRIVATE include)

add_executable(test_reliable tests/test_reliable.c src/t425_reliable.c src/t425_frame.c)
target_include_directories(test_reliable PRIVATE include)

add_executable(test_link_stack tests/test_link_stack.c src/t425_link.c src/t425_reliable.c src/t425_frame.c)
target_include_directories(test_link_stack PRIVATE include)

add_executable(test_v28_boundary tests/test_v28_boundary.c src/t425_link.c src/t425_reliable.c src/t425_frame.c)
target_include_directories(test_v28_boundary PRIVATE include)

add_executable(test_stream tests/test_stream.c src/t425_stream.c src/t425_frame.c)
target_include_directories(test_stream PRIVATE include)

add_executable(test_ack tests/test_ack.c src/t425_ack.c)
target_include_directories(test_ack PRIVATE include)

add_executable(test_node_config tests/test_node_config.c src/t425_node_config.c)
target_include_directories(test_node_config PRIVATE include)

add_executable(test_runtime tests/test_runtime.c src/t425_runtime.c src/t425_node_config.c src/t425_stream.c src/t425_frame.c src/t425_ack.c)
target_include_directories(test_runtime PRIVATE include)

add_executable(test_supervisor tests/test_supervisor.c src/t425_supervisor.c src/t425_scheduler.c src/t425_runtime.c src/t425_node_config.c src/t425_stream.c src/t425_frame.c src/t425_ack.c)
target_include_directories(test_supervisor PRIVATE include)
