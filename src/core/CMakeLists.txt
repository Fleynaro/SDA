
file(GLOB_RECURSE CORE_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(
    core STATIC
    ${CORE_SRC}
)

target_include_directories(
    core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

target_link_libraries(
    core PUBLIC
    utilities
)

foreach(_source IN ITEMS ${CORE_SRC})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_group_path}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()