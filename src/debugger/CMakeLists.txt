
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
    debugger PRIVATE
    utilities
    magic_enum::magic_enum
    Zydis)

create_vs_filters("${DEBUGGER_SRC}")