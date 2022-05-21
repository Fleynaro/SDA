if(WIN32)
    option(ZYDIS_FEATURE_ENCODER OFF)
    option(ZYDIS_BUILD_EXAMPLES OFF)
    option(ZYDIS_BUILD_TOOLS OFF)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third-party/zydis)
else()
    find_package(zydis CONFIG REQUIRED)
endif()

file(GLOB_RECURSE DISASM_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(
    disasm SHARED ${DISASM_SRC})

target_include_directories(
    disasm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(
    disasm PUBLIC
    core
    Zydis)