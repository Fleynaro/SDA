
cmake_minimum_required(VERSION 3.1)
project(sda)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# all options
option(USE_TESTS "Use unit tests" ON)

include_directories(src)
add_subdirectory(src)

# for creating the gui app
add_executable(
    sda_gui
    src/main.cpp
    src/main.h
    )
target_link_libraries(
    sda_gui PRIVATE
    core
    gui
    )

# for creating the console app to test
if(USE_TESTS)
    file(GLOB_RECURSE TEST_SRC
        tests/*.cpp
        tests/*.h)
    add_executable(
        sda_tests
        ${TEST_SRC}
        )
    target_link_libraries(
        sda_tests PRIVATE
        core
        )
endif()