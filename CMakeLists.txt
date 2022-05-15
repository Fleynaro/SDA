cmake_minimum_required(VERSION 3.1)
project(sda)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Import cmake files
include(cmake/compiler_flags.cmake)

# Packages:
find_package(Boost REQUIRED COMPONENTS system filesystem json)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

add_subdirectory(core)
add_subdirectory(sda)
if (NOT SDA_PLUGIN_DEV)
    add_subdirectory(launcher)
endif()