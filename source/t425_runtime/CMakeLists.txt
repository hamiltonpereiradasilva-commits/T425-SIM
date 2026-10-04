add_library(t425_core_phy_service STATIC
    ${CMAKE_CURRENT_LIST_DIR}/../t425_phy/rp2040/core_adapter/t425_core_phy_ipc.c
    ${CMAKE_CURRENT_LIST_DIR}/../t425_phy/rp2040/core_adapter/t425_core_phy_bootstrap.c
)
target_include_directories(t425_core_phy_service PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../t425_core/phy
    ${CMAKE_CURRENT_LIST_DIR}/../t425_phy/rp2040/core_adapter
)
target_link_libraries(t425_core_phy_service PUBLIC
    t425_core_phy_contract
    pico_util
)
add_library(t425_runtime STATIC t425_runtime.c)
target_include_directories(t425_runtime PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(t425_runtime PUBLIC
    t425_core_phy_service
    t425_core_phy_contract
)
