file(GLOB_RECURSE CORE_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(
    core SHARED ${CORE_SRC})

target_include_directories(
    core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(
    core PUBLIC shared)