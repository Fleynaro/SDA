file(GLOB_RECURSE PLUGIN_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(
    plugin SHARED ${PLUGIN_SRC})

target_include_directories(
    plugin PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(
    plugin PUBLIC
    core
    Boost::boost
    Boost::system Boost::filesystem)