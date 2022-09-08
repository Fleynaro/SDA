find_package(
    magic_enum CONFIG REQUIRED)

file(GLOB_RECURSE BINDINGS_JS_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(
    bindings-js SHARED ${CMAKE_JS_SRC} ${BINDINGS_JS_SRC})

target_include_directories(
    bindings-js PUBLIC
    ${CMAKE_JS_INC}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/v8pp)

target_link_libraries(
    bindings-js PUBLIC
    ${CMAKE_JS_LIB}
    magic_enum::magic_enum)

target_compile_options(
    bindings-js PUBLIC /w44275 /w44251)