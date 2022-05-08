file(GLOB_RECURSE SDA_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(
    sda SHARED ${SDA_SRC})

target_include_directories(
    sda PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(
    sda PUBLIC
    core
    Boost::boost
    Boost::system Boost::filesystem)