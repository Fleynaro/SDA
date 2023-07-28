cmake_minimum_required(VERSION 3.1)
project(sda)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

option(BUILD_TESTING "Build and run tests" OFF)

# Import cmake files
include(cmake/compiler_flags.cmake)

if(BUILD_TESTING)
    find_package(
        GTest CONFIG REQUIRED)
    include(GoogleTest)
    enable_testing()
endif()

add_subdirectory(core)
add_subdirectory(platform)
if (DEFINED CMAKE_JS_VERSION)
    message(STATUS "Building for node.js (cmake.js ${CMAKE_JS_VERSION})")
    add_subdirectory(javascript/packages)
endif()