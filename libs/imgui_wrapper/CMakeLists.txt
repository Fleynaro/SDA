cmake_minimum_required(VERSION 3.1)
project(imgui_wrapper)

set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/external/imgui")
set(IMGUI_TEXT_EDITOR_DIR "${CMAKE_SOURCE_DIR}/external/ImGuiColorTextEdit")

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
# imgui text editor
file(GLOB IMGUI_TEXT_EDITOR_SRC
    ${IMGUI_TEXT_EDITOR_DIR}/*.cpp
    ${IMGUI_TEXT_EDITOR_DIR}/*.h)
# imgui wrapper
file(GLOB_RECURSE IMGUI_WRAPPER_SRC
    "include/*.h"
    "src/*.cpp")

add_library(
    imgui_wrapper STATIC
    ${IMGUI_WRAPPER_SRC}
    ${IMGUI_SRC}
    ${IMGUI_BACKEND_SRC}
    ${IMGUI_MISC_SRC}
    ${IMGUI_TEXT_EDITOR_SRC})

target_include_directories(
    imgui_wrapper PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${IMGUI_DIR}
    ${IMGUI_TEXT_EDITOR_DIR})

target_compile_definitions(
    imgui_wrapper PRIVATE
    IMGUI_DISABLE_OBSOLETE_FUNCTIONS=1)

create_vs_filters("${IMGUI_WRAPPER_SRC}")