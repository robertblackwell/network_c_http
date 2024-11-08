set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/async.h
    ${CMAKE_CURRENT_SOURCE_DIR}/async_socket.h
    ${CMAKE_CURRENT_SOURCE_DIR}/connection_internal.h
)
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/async_handler.c
    ${CMAKE_CURRENT_SOURCE_DIR}/async_socket.c
    ${CMAKE_CURRENT_SOURCE_DIR}/async_server.c
    ${CMAKE_CURRENT_SOURCE_DIR}/connection_api.c
    ${CMAKE_CURRENT_SOURCE_DIR}/connection_cleanup.c
    ${CMAKE_CURRENT_SOURCE_DIR}/connection_event.c
    ${CMAKE_CURRENT_SOURCE_DIR}/connection_helpers.c
    ${CMAKE_CURRENT_SOURCE_DIR}/connection_read.c
    ${CMAKE_CURRENT_SOURCE_DIR}/connection_write.c
)

set_source_files_properties(${SOURCE_FILES} PROPERTIES LANGUAGE C)
set(C_HTTP_LIBRARY_SOURCE_FILES ${C_HTTP_LIBRARY_SOURCE_FILES} ${SOURCE_FILES} PARENT_SCOPE)
set(C_HTTP_LIBRARY_HEADER_FILES ${C_HTTP_LIBRARY_HEADER_FILES} ${HEADER_FILES} PARENT_SCOPE)

