
cmake_minimum_required(VERSION 3.1)
project(imgui_wrapper)

find_package(imgui CONFIG REQUIRED)

file(GLOB_RECURSE IMGUI_WRAPPER_SRC
    include/*.h
    src/*.cpp)

add_library(
    imgui_wrapper STATIC
    ${IMGUI_WRAPPER_SRC})

target_include_directories(
    imgui_wrapper PUBLIC
    ${CMAKE_SOURCE_DIR}/include)
    
target_link_libraries(
    imgui_wrapper PUBLIC
    imgui::imgui)