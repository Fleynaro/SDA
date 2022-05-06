add_executable(
    sda
    ${CMAKE_CURRENT_SOURCE_DIR}/launcher.cpp)

target_link_libraries(
    sda PRIVATE core database)

if(CMAKE_DL_LIBS)
    target_link_libraries(sda PRIVATE ${CMAKE_DL_LIBS})
endif()