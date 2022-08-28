file(GLOB_RECURSE SDA_TEST
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(
    sda-test ${SDA_TEST})

target_link_libraries(
    sda-test PUBLIC
    sda-test-common)

gtest_discover_tests(sda-test)