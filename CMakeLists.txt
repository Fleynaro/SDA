
cmake_minimum_required(VERSION 3.1)
project(sda)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# packages:
find_package(Boost REQUIRED COMPONENTS system filesystem)

add_subdirectory(core)
add_subdirectory(plugin)
add_subdirectory(launcher)