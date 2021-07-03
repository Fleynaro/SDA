# compiler flags for visual studio
if(MSVC)
    # parallel build
    add_definitions(/MP)
    # ignore some warnings (/w44250 - make C4250 warning belong to 4-level)
    add_compile_options(/W3 /w44250)
endif()