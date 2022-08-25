file(GLOB_RECURSE CORE_TEST
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(
    core-test ${CORE_TEST})

target_link_libraries(
    core-test PUBLIC
    core-test-common)

gtest_discover_tests(core-test)