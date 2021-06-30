
file(GLOB_RECURSE GUI_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(
    gui STATIC
    ${GUI_SRC}
)

target_link_libraries(
    gui PUBLIC
    core
)