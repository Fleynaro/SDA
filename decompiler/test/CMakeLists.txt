file(GLOB_RECURSE DECOMPILER_TEST
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(
    decompiler-test ${DECOMPILER_TEST})

target_include_directories(
    decompiler-test PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
    decompiler-test PUBLIC
    decompiler
    GTest::gtest)

gtest_discover_tests(decompiler-test)