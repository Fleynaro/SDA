
cmake_minimum_required(VERSION 3.1)
project(dec_test_samples)

file(GLOB_RECURSE DEC_TEST_SAMPLES_SRC *.cpp *.h)

add_library(
    dec_test_samples STATIC
    ${DEC_TEST_SAMPLES_SRC})

target_include_directories(
    dec_test_samples PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
    dec_test_samples PRIVATE
    dec_test_functions
    core)