cmake_minimum_required(VERSION 3.1)
project(sda-plugin)

# Path to the root of the SDA project
set(SDA_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)

# Paths to headers and sources of the plugin
file(GLOB_RECURSE PLUGIN_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)


if (NOT SDA_DEV)
    # Import the SDA project
    set(SDA_PLUGIN_DEV ON)
    add_subdirectory(${SDA_ROOT_DIR} ${CMAKE_CURRENT_BINARY_DIR}/sda)
endif()

# Create shared library
add_library(
    myplugin SHARED ${PLUGIN_SRC})

# Set include paths
target_include_directories(
    myplugin PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Link the plugin to the SDA library
target_link_libraries(
    myplugin PUBLIC sda)