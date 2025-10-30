set(HEADER_FILES
        msg_stream.h

)
set(SOURCE_FILES
        msg_reader.c
        msg_stream.c
        msg_writer.c
)

set_source_files_properties(${SOURCE_FILES} PROPERTIES LANGUAGE C)
set(C_HTTP_LIBRARY_SOURCE_FILES ${C_HTTP_LIBRARY_SOURCE_FILES} ${SOURCE_FILES} PARENT_SCOPE)
set(C_HTTP_LIBRARY_HEADER_FILES ${C_HTTP_LIBRARY_HEADER_FILES} ${HEADER_FILES} PARENT_SCOPE)

