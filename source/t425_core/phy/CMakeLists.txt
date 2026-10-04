add_library(t425_core_phy_contract STATIC
    t425_core_phy.c
)
target_include_directories(t425_core_phy_contract PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)
