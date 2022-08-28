file(GLOB_RECURSE SDA_TEST_COMMON
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(
    sda-test-common SHARED ${SDA_TEST_COMMON})

target_include_directories(
    sda-test-common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
    sda-test-common PUBLIC
    core-test-common
    sda)