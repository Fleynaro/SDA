file(GLOB_RECURSE CORE_JS_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(
    core-js SHARED ${CMAKE_JS_SRC} ${CORE_JS_SRC})

target_include_directories(
    core-js PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src)

set_target_properties(
    core-js PROPERTIES PREFIX "" SUFFIX ".node")

target_link_libraries(
    core-js PUBLIC
    bindings-js
    core)