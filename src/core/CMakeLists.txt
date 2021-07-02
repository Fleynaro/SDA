
file(GLOB_RECURSE CORE_SRC
    ${CMAKE_SOURCE_DIR}/src/core/*.cpp
    ${CMAKE_SOURCE_DIR}/src/core/*.h
    ${CMAKE_SOURCE_DIR}/src/database/*.cpp
    ${CMAKE_SOURCE_DIR}/src/database/*.h
    ${CMAKE_SOURCE_DIR}/src/decompiler/*.cpp
    ${CMAKE_SOURCE_DIR}/src/decompiler/*.h
    ${CMAKE_SOURCE_DIR}/src/ghidra_sync/*.cpp
    ${CMAKE_SOURCE_DIR}/src/ghidra_sync/*.h)

add_library(
    core STATIC
    ${CORE_SRC}
)

target_include_directories(
    core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

target_link_libraries(
    core PUBLIC
    utilities
    ghidra_sync_thrift
    nlohmann_json
    magic_enum::magic_enum
    SQLiteCpp
    asmjit
    asmtk
    minhook::minhook
    Zydis
)

create_vs_filters("${CORE_SRC}")