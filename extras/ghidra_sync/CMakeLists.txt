
set(THRIFT_SRC_DIR ${CMAKE_CURRENT_BINARY_DIR}/src)
set(THRIFT_SERVICES
    "DataSyncPacketManagerService")
thrift_generate_cpp(
    THRIFT_SRCS
    THRIFT_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${THRIFT_SRC_DIR}/ghidra_sync
    ghidra.thrift
    "${THRIFT_SERVICES}"
)

add_library(
    ghidra_sync_thrift STATIC
    ${THRIFT_SRCS}
    ${THRIFT_HDRS}
)

target_include_directories(
    ghidra_sync_thrift
    PRIVATE
    ${THRIFT_INCLUDE_DIR}/..
    PUBLIC
    ${THRIFT_SRC_DIR}
    )

target_link_libraries(
    ghidra_sync_thrift PUBLIC
    thrift::thrift
)