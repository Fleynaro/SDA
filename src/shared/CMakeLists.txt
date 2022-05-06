file(GLOB_RECURSE SHARED_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(
    shared SHARED ${SHARED_SRC})

target_include_directories(
    shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(
    shared PUBLIC Boost::boost Boost::system Boost::filesystem)