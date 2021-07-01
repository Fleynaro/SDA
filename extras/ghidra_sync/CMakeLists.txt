
set(THRIFT_SERVICES
    "DataSyncPacketManagerService")
thrift_generate_cpp(
    THRIFT_SRCS
    THRIFT_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ghidra.thrift
    "${THRIFT_SERVICES}"
)

add_library(
    ghidra_sync_thrift STATIC
    ${THRIFT_SRCS}
    ${THRIFT_HDRS}
)

target_link_libraries(
    ghidra_sync_thrift PUBLIC
    thrift::thrift
)