file(GLOB_RECURSE SDA_JS_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(
    sda-js SHARED ${CMAKE_JS_SRC} ${SDA_JS_SRC})

target_include_directories(
    sda-js PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src)

set_target_properties(
    sda-js PROPERTIES PREFIX "" SUFFIX ".node")

target_link_libraries(
    sda-js PUBLIC
    bindings-js
    sda)