file(GLOB_RECURSE DEC_TEST_COMMON
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(
    decompiler-test-common SHARED ${DEC_TEST_COMMON})

target_include_directories(
    decompiler-test-common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
    decompiler-test-common PUBLIC
    core-test-common
    decompiler)