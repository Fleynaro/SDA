
cmake_minimum_required(VERSION 3.1)
project(decompiler_test_lib)

file(GLOB_RECURSE DEC_TEST_LIB_SRC *.cpp *.h)

add_library(
    decompiler_test_lib STATIC
    ${DEC_TEST_LIB_SRC}
)