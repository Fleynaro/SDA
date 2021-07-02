
cmake_minimum_required(VERSION 3.1)
project(sda)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Using vcpkg you should install the required libraries.
# Command: vcpkg install nlohmann-json magic-enum sqlitecpp gtest zydis asmjit thrift minhook imgui --triplet x64-windows
# Build command: cmake .. -G "Visual Studio 16 2019" -A x64 "-DCMAKE_TOOLCHAIN_FILE=C:\src\vcpkg\scripts\buildsystems\vcpkg.cmake"
# packages:
find_package(nlohmann_json CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(SQLiteCpp CONFIG REQUIRED)
find_package(Thrift CONFIG REQUIRED)
find_package(minhook CONFIG REQUIRED)
if(WIN32)
    option(ZYDIS_FEATURE_KNC OFF)
    option(ZYDIS_BUILD_EXAMPLES OFF)
    option(ZYDIS_BUILD_TOOLS OFF)
    add_subdirectory(external/zydis)
    set_target_properties(Zycore PROPERTIES FOLDER "external")
    set_target_properties(Zydis PROPERTIES FOLDER "external")
else()
    find_package(zydis CONFIG REQUIRED)
endif()
add_subdirectory(external/asmtk)
set_target_properties(asmjit PROPERTIES FOLDER "external")
set_target_properties(asmtk PROPERTIES FOLDER "external")

# all options
option(USE_TESTS "Use unit tests" ON)

# import functions
include(cmake/func_vs_filters.cmake)
include(cmake/func_thrift_generator.cmake)

add_subdirectory(libs/imgui_wrapper)
set_target_properties(imgui_wrapper PROPERTIES FOLDER "libs")
add_subdirectory(extras/ghidra_sync)
include_directories(src)
add_subdirectory(src)

# for creating the gui app
add_executable(
    sda_gui
    src/gui/main.cpp
    )
target_link_libraries(
    sda_gui PRIVATE
    core
    gui
    )

# for creating the console app to test
if(USE_TESTS)
    find_package(GTest CONFIG REQUIRED)
    add_subdirectory(extras/decompiler_test_lib)
    set_target_properties(decompiler_test_lib PROPERTIES FOLDER "extras")

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
        decompiler_test_lib
        GTest::gtest
        )
    create_vs_filters("${TEST_SRC}")
endif()