##
## marvin/CMakeLists.txt
##
## this directory exports marvin_library
##
## Note includes 4 files from vendor/src
##
set(EXTSRC ${C_HTTP_VENDOR_SRCDIR})

#add_custom_command(
#	# generate mytypet list code
#	COMMAND bash ${C_HTTP_TOOLS_DIR}/gen_typed_list.sh generate MyType MT
#	DEPENDS list/template.h list/template.c mytype/hand_coded.h mytype/hand_coded.c
#	OUTPUT ${C_HTTP_SOURCE_DIR}/mytype_list.h ${C_HTTP_SOURCE_DIR}/mytype_list.c
#	COMMENT "Generating mytype_list.h mytype_list.c"
#	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
#)
#add_custom_command(
#	# generate kvpair list code
#	COMMAND bash ${C_HTTP_TOOLS_DIR}/gen_typed_list.sh generate KVPair HDR
#	DEPENDS list/template.h list/template.c headerline/hand_coded.h headerline/hand_coded.c
#	OUTPUT ${C_HTTP_SOURCE_DIR}/hdrlist.h ${C_HTTP_SOURCE_DIR}/hdrlist.c
#	COMMENT "Generating kvpair_list.h kvpair_list.c"
#	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
#)

set(C_HTTP_LIBRARY_HEADER_FILES
	#basic data structures
	#misc
        ../rbl/logger.h
		../rbl/unittest.h
		common/socket_functions.h
		../rbl/check_tag.h
		${EXTSRC}/llhttp/llhttp.h
	)

set(C_HTTP_LIBRARY_SOURCE_FILES
		../rbl/unittest.c
        ../rbl/logger.c
		common/socket_functions.c
		${EXTSRC}/llhttp/api.c
		${EXTSRC}/llhttp/http.c
		${EXTSRC}/llhttp/llhttp.c
	#async
#		async/async_handler.c
#		async/async_socket.c
#		async/async_server.c
#        async/connection_api.c
#		async/connection_cleanup.c
#		async/connection_event.c
#		async/connection_helpers.c
#		async/connection_read.c
#		async/connection_write.c
	)

set_source_files_properties(${C_HTTP_LIBRARY_SOURCE_FILES} PROPERTIES LANGUAGE C)
add_subdirectory(http_protocol)
add_subdirectory(common)
add_subdirectory(demo_protocol)
add_subdirectory(http)
add_subdirectory(runloop)
add_subdirectory(sync)
foreach(F in ${C_HTTP_LIBRARY_SOURCE_FILES})
	message("SOURCE FILE: ${F}")
endforeach()
foreach(F in ${C_HTTP_LIBRARY_HEADER_FILES})
	message("HEADER FILE: ${F}")
endforeach()

add_library(${C_HTTP_LIB_NAME}
	STATIC 
		${C_HTTP_LIBRARY_SOURCE_FILES} 
		${C_HTTP_LIBRARY_HEADER_FILES}
)
target_compile_definitions(${C_HTTP_LIB_NAME} PUBLIC RBL_TAG_CHECK_ON=1)
target_compile_definitions(${C_HTTP_LIB_NAME} PUBLIC RUNLOOP_ITQUEUE_ENABLE=1)
target_include_directories(${C_HTTP_LIB_NAME}
	SYSTEM PUBLIC 
	${C_HTTP_INCLUDE_PATHS}
)
