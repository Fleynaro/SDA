file(GLOB_RECURSE PLATFORM_TEST_COMMON
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(
    platform-x86-test-common SHARED ${PLATFORM_TEST_COMMON})

target_include_directories(
    platform-x86-test-common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
    platform-x86-test-common PUBLIC
    core-test-common
    platform-x86)