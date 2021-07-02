cmake_minimum_required(VERSION 3.1)
project(imgui_wrapper)

set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/external/imgui")

# imgui
file(GLOB IMGUI_SRC
    ${IMGUI_DIR}/*.cpp
    ${IMGUI_DIR}/*.h)
set(IMGUI_BACKEND_SRC
    ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp
    ${IMGUI_DIR}/backends/imgui_impl_dx11.h
    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
    ${IMGUI_DIR}/backends/imgui_impl_win32.h)
set(IMGUI_MISC_SRC
    ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
    ${IMGUI_DIR}/misc/cpp/imgui_stdlib.h)
# imgui wrapper
file(GLOB_RECURSE IMGUI_WRAPPER_SRC
    "include/*.h"
    "src/*.cpp")

add_library(
    imgui_wrapper STATIC
    ${IMGUI_WRAPPER_SRC}
    ${IMGUI_SRC}
    ${IMGUI_BACKEND_SRC}
    ${IMGUI_MISC_SRC})

target_include_directories(
    imgui_wrapper PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${IMGUI_DIR})

create_vs_filters("${IMGUI_WRAPPER_SRC}")