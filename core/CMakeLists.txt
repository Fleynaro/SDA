find_package(
    Boost REQUIRED COMPONENTS
    system
    filesystem
    json)
find_package(
    plog CONFIG REQUIRED)
find_package(
    magic_enum CONFIG REQUIRED)
find_path(
    RANG_INCLUDE_DIRS "rang.hpp")

file(GLOB_RECURSE CORE_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(
    core SHARED ${CORE_SRC})

target_include_directories(
    core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
    ${RANG_INCLUDE_DIRS})

target_link_libraries(
    core PUBLIC
    Boost::boost
    Boost::system
    Boost::filesystem
    Boost::json
    plog::plog
    magic_enum::magic_enum)

if(BUILD_TESTING)
	add_subdirectory(test)
endif()