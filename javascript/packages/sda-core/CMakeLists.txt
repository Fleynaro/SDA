file(GLOB_RECURSE CORE_JS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(
    core-js SHARED ${CORE_JS})

set_target_properties(
    core-js PROPERTIES PREFIX "" SUFFIX ".node")

target_link_libraries(
    core-js PUBLIC
    bindings-js
    core)