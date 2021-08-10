
cmake_minimum_required(VERSION 3.1)
project(test_program)

file(GLOB_RECURSE TEST_PROGRAM_SRC *.cpp *.h)

add_executable(
    test_program
    ${TEST_PROGRAM_SRC})

target_include_directories(
    test_program PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})