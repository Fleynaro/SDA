file(GLOB_RECURSE CORE_TEST_COMMON
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(
    core-test-common SHARED ${CORE_TEST_COMMON})

target_include_directories(
    core-test-common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
    core-test-common PUBLIC
    core
    GTest::gmock_main)