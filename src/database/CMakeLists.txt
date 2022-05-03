file(GLOB_RECURSE DATABASE_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(
    database SHARED ${DATABASE_SRC})

target_include_directories(
    database PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(
    database PUBLIC core)