file(GLOB_RECURSE DEC_TEST
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(
    decompiler-test ${DEC_TEST})

target_link_libraries(
    decompiler-test PUBLIC
    decompiler-test-common)

gtest_discover_tests(decompiler-test)