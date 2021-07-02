
file(GLOB_RECURSE GUI_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
list(REMOVE_ITEM GUI_SRC "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

add_library(
    gui STATIC
    ${GUI_SRC}
)

target_include_directories(
    gui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

target_link_libraries(
    gui PUBLIC
    core
    imgui_wrapper
)

create_vs_filters("${GUI_SRC}")