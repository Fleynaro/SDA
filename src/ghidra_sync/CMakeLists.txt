
file(GLOB_RECURSE GHIDRA_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

target_sources(
    core PRIVATE
    ${GHIDRA_SRC}
)