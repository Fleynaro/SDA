find_package(
    magic_enum CONFIG REQUIRED)

file(GLOB_RECURSE BINDINGS_JS_SHARED_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Shared/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Shared/*.cpp)

add_library(
    bindings-js-shared SHARED ${BINDINGS_JS_SHARED_SRC})

file(GLOB_RECURSE BINDINGS_JS_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Bind/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Bind/*.cpp)

add_library(
    bindings-js STATIC ${BINDINGS_JS_SRC})

target_include_directories(
    bindings-js PUBLIC
    ${CMAKE_JS_INC}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/v8pp)

target_link_libraries(
    bindings-js PUBLIC
    bindings-js-shared
    ${CMAKE_JS_LIB}
    magic_enum::magic_enum)

target_compile_options(
    bindings-js PUBLIC /bigobj /w44275 /w44251)