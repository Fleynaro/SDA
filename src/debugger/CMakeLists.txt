
file(GLOB_RECURSE DEBUGGER_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(
    debugger STATIC
    ${DEBUGGER_SRC})

target_include_directories(
    debugger PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
    debugger PUBLIC
    utilities)

create_vs_filters("${DEBUGGER_SRC}")