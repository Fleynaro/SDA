
cmake_minimum_required(VERSION 3.1)
project(sda)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# import cmake files
include(cmake/func_vs_filters.cmake)
include(cmake/func_thrift_generator.cmake)
include(cmake/compiler_flags.cmake)
include(cmake/CMakeRC.cmake)

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

add_subdirectory(libs/imgui_wrapper)
set_target_properties(imgui_wrapper PROPERTIES FOLDER "libs")
add_subdirectory(extras/ghidra_sync)
add_subdirectory(data/resources)
include_directories(src)
add_subdirectory(src/utilities)
add_subdirectory(src/debugger)
add_subdirectory(src/core)
add_subdirectory(extras/dec_test_functions)
add_subdirectory(extras/dec_test_samples)
add_subdirectory(extras/test_program)
add_subdirectory(src/gui)

set_target_properties(dec_test_samples PROPERTIES FOLDER "extras")
set_target_properties(dec_test_functions PROPERTIES FOLDER "extras")
set_target_properties(test_program PROPERTIES FOLDER "extras")

# for creating the gui app
add_executable(
    sda_gui WIN32
    src/gui/main.cpp
    )
target_link_libraries(
    sda_gui PRIVATE
    core
    gui
    d3d11.lib
    )

# for creating the console app to test
if(USE_TESTS)
    find_package(GTest CONFIG REQUIRED)

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
        dec_test_samples
        GTest::gtest
        )
    target_compile_definitions(
        sda_tests PRIVATE TEST_DATA_PATH="${CMAKE_SOURCE_DIR}/data/test")
    create_vs_filters("${TEST_SRC}")
endif()
