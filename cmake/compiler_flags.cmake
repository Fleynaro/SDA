# compiler flags for visual studio
if(MSVC)
    # parallel build
    add_definitions(/MP)
    
    # ignore some warnings (/w44250 - make C4250 warning belong to 4-level)
    add_compile_options(/W3 /w44250)

    # ignore some linker warnings
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4199")

    # for boost uuid
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_WIN32_WINNT=0x0601 -DBOOST_UUID_FORCE_AUTO_LINK")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D_WIN32_WINNT=0x0601 -DBOOST_UUID_FORCE_AUTO_LINK")
endif()