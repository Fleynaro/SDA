file(GLOB_RECURSE PLATFORM_TEST
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(
    platform-x86-test ${PLATFORM_TEST})

target_link_libraries(
    platform-x86-test PUBLIC
    platform-x86-test-common)

gtest_discover_tests(platform-x86-test)