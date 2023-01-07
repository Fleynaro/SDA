file(GLOB_RECURSE DECOMPILER_JS_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(
    decompiler-js SHARED ${CMAKE_JS_SRC} ${DECOMPILER_JS_SRC})

target_include_directories(
    decompiler-js PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src)

set_target_properties(
    decompiler-js PROPERTIES PREFIX "" SUFFIX ".node")

target_link_libraries(
    decompiler-js PUBLIC
    bindings-js
    core-shared-js
    decompiler)