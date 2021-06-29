
cmake_minimum_required(VERSION 3.0)
project(sda)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(csv-parser)

add_executable(
    labcode2
    src/main.cpp
    src/main.h
    )

target_include_directories(
    labcode2 PUBLIC
    csv-parser/single_include
    )

target_link_libraries(
    labcode2
    csv
    )