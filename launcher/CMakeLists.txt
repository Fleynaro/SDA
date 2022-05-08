add_executable(
    launcher
    ${CMAKE_CURRENT_SOURCE_DIR}/launcher.cpp)

target_link_libraries(
    launcher PRIVATE sda)

if(CMAKE_DL_LIBS)
    target_link_libraries(launcher PRIVATE ${CMAKE_DL_LIBS})
endif()