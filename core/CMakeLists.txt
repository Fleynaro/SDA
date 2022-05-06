file(GLOB_RECURSE CORE_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(
    core SHARED ${CORE_SRC})

target_include_directories(
    core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)