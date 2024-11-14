set(HEADER_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/http_sync_socket.h
        ${CMAKE_CURRENT_SOURCE_DIR}/http_connection.h
		${CMAKE_CURRENT_SOURCE_DIR}/http_handler.h
        ${CMAKE_CURRENT_SOURCE_DIR}/http_message.h
        ${CMAKE_CURRENT_SOURCE_DIR}/http_parser.h
		${CMAKE_CURRENT_SOURCE_DIR}/http_server.h
		${CMAKE_CURRENT_SOURCE_DIR}/http_process_request.h
#		${CMAKE_CURRENT_SOURCE_DIR}/http_sync_reader.h
#		${CMAKE_CURRENT_SOURCE_DIR}/http_sync_reader_private.h
#		${CMAKE_CURRENT_SOURCE_DIR}/http_sync_writer.h
)
set(SOURCE_FILES
		${CMAKE_CURRENT_SOURCE_DIR}/http_sync_socket.c
	${CMAKE_CURRENT_SOURCE_DIR}/http_connection.c
	${CMAKE_CURRENT_SOURCE_DIR}/http_handler.c
	${CMAKE_CURRENT_SOURCE_DIR}/http_message.c
	${CMAKE_CURRENT_SOURCE_DIR}/http_parser.c
	${CMAKE_CURRENT_SOURCE_DIR}/http_process_request.c
	${CMAKE_CURRENT_SOURCE_DIR}/http_server.c
#	${CMAKE_CURRENT_SOURCE_DIR}/http_parser_test.c
#	${CMAKE_CURRENT_SOURCE_DIR}/http_sync_reader.c
#	${CMAKE_CURRENT_SOURCE_DIR}/http_sync_writer.c

)

set_source_files_properties(${SOURCE_FILES} PROPERTIES LANGUAGE C)
set(C_HTTP_LIBRARY_SOURCE_FILES ${C_HTTP_LIBRARY_SOURCE_FILES} ${SOURCE_FILES} PARENT_SCOPE)
set(C_HTTP_LIBRARY_HEADER_FILES ${C_HTTP_LIBRARY_HEADER_FILES} ${HEADER_FILES} PARENT_SCOPE)

