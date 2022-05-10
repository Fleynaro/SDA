file(GLOB_RECURSE CORE_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(
    core SHARED ${CORE_SRC})

target_include_directories(
    core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS})

target_link_libraries(
    core PUBLIC
    Boost::boost
    Boost::system
    Boost::filesystem
    Boost::json)