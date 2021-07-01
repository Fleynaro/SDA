# generate .h, .cpp files from .thrift files
function(thrift_generate_cpp SRCS HDRS I_DIR O_DIR I_MAIN_FILE SERVICES)
    # iterate over .thrift files
    file(GLOB I_FILES "${I_DIR}/*.thrift")
    foreach(I_FILE ${I_FILES})
        # get components from path (abs. path and name without ext.)
        get_filename_component(ABS_FIL ${I_FILE} ABSOLUTE)
        get_filename_component(FIL_WE ${I_FILE} NAME_WE)
        # create target file paths (.cpp, .h)
        list(APPEND SRCS "${O_DIR}/${FIL_WE}_types.h" "${O_DIR}/${FIL_WE}_constants.h")
        list(APPEND HDRS "${O_DIR}/${FIL_WE}_types.cpp" "${O_DIR}/${FIL_WE}_constants.cpp")

        # message("file: " ${FIL_WE})
    endforeach()
    
    # iterate over services
    foreach(SERVICE ${SERVICES})
        list(APPEND SRCS "${O_DIR}/${SERVICE}.h")
        list(APPEND HDRS "${O_DIR}/${SERVICE}.cpp")
    endforeach()

    add_custom_command(
        OUTPUT
            ${SRCS}
            ${HDRS}
        COMMAND
            "${CMAKE_SOURCE_DIR}/tools/thrift.exe"
        ARGS
            --gen cpp:no_skeleton -r -out ${O_DIR} ${I_DIR}/${I_MAIN_FILE}
        DEPENDS
            ${I_FILES}
        VERBATIM
        )

    # mark target files as generated during compilation
    set_source_files_properties(${SRCS} ${HDRS} PROPERTIES GENERATED TRUE)

    # return lists into the out arguments
    set(${SRCS} PARENT_SCOPE)
    set(${HDRS} PARENT_SCOPE)
endfunction()