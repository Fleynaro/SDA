
cmake_minimum_required(VERSION 3.1)
project(dec_test_functions)

file(GLOB_RECURSE DEC_TEST_FUNC_SRC *.cpp *.h)

add_library(
    dec_test_functions STATIC
    ${DEC_TEST_FUNC_SRC})

target_include_directories(
    dec_test_functions PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})