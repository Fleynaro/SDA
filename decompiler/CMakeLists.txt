file(GLOB_RECURSE DECOMPILER_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(
    decompiler SHARED ${DECOMPILER_SRC})

target_include_directories(
    decompiler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(
    decompiler PUBLIC
    core)

if(BUILD_TESTING)
	add_subdirectory(test)
endif()