
cmake_minimum_required(VERSION 3.1)
project(decompiler_test_lib)

file(GLOB_RECURSE DEC_TEST_LIB_SRC *.cpp *.h)

add_library(
    decompiler_test_lib STATIC
    ${DEC_TEST_LIB_SRC})

target_include_directories(
    decompiler_test_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(
    decompiler_test_lib PRIVATE
    core)