file(GLOB_RECURSE PLATFORM_JS_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(
    platform-x86-js SHARED ${PLATFORM_JS_SRC})

target_include_directories(
    platform-x86-js PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src)

set_target_properties(
    platform-x86-js PROPERTIES PREFIX "" SUFFIX ".node")

target_link_libraries(
    platform-x86-js PUBLIC
    bindings-js
    platform-x86)