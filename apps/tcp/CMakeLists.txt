set(HEADER_FILES
        tcp_stream.h
        tcp_stream_internal.h

)
set(SOURCE_FILES
        tcp_listener.c
        tcp_reader.c
        tcp_stream.c
        tcp_writer.c
)

set_source_files_properties(${SOURCE_FILES} PROPERTIES LANGUAGE C)
set(C_HTTP_LIBRARY_SOURCE_FILES ${C_HTTP_LIBRARY_SOURCE_FILES} ${SOURCE_FILES} PARENT_SCOPE)
set(C_HTTP_LIBRARY_HEADER_FILES ${C_HTTP_LIBRARY_HEADER_FILES} ${HEADER_FILES} PARENT_SCOPE)

