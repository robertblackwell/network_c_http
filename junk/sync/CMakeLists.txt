set(HEADER_FILES
        sync.h
        sync_internal.h
        tags.h
        sync_client.h

)
set(SOURCE_FILES
        sync_client.c
        sync_connection.c
        sync_server.c
        sync_handler_example.c
        worker.c
)

set_source_files_properties(${SOURCE_FILES} PROPERTIES LANGUAGE C)
set(C_HTTP_LIBRARY_SOURCE_FILES ${C_HTTP_LIBRARY_SOURCE_FILES} ${SOURCE_FILES} PARENT_SCOPE)
set(C_HTTP_LIBRARY_HEADER_FILES ${C_HTTP_LIBRARY_HEADER_FILES} ${HEADER_FILES} PARENT_SCOPE)

