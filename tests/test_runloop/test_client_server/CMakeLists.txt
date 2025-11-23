

set(target test_client_server_original)
set(TARGET_SRC
		test_client_server.c
		${C_HTTP_PROJECT_DIR}/apps/simple_request_response_app/simple_app.h
		${C_HTTP_PROJECT_DIR}/apps/simple_request_response_app/simple_app.c

		${C_HTTP_PROJECT_DIR}/apps/server/server_ctx.h
		${C_HTTP_PROJECT_DIR}/apps/server/server_ctx.c
		${C_HTTP_PROJECT_DIR}/apps/server/server_memory.h
		${C_HTTP_PROJECT_DIR}/apps/server/server_memory.c

#		${C_HTTP_PROJECT_DIR}/apps/tcp/tcp_stream.h
#		${C_HTTP_PROJECT_DIR}/apps/tcp/tcp_stream_internal.h
#		${C_HTTP_PROJECT_DIR}/apps/tcp/tcp_stream.c
#		${C_HTTP_PROJECT_DIR}/apps/tcp/tcp_reader.c
#		${C_HTTP_PROJECT_DIR}/apps/tcp/tcp_writer.c
#		${C_HTTP_PROJECT_DIR}/apps/tcp/tcp_listener.c

		${C_HTTP_PROJECT_DIR}/apps/msg/msg_generic.h
		${C_HTTP_PROJECT_DIR}/apps/msg/msg_generic.c

#		${C_HTTP_PROJECT_DIR}/src/newline/newline_msg.h
#		${C_HTTP_PROJECT_DIR}/src/newline/newline_msg.c
#		${C_HTTP_PROJECT_DIR}/src/newline/newline_msg_parser.c
#
#		${C_HTTP_PROJECT_DIR}/src/stx/stx_msg.h
#		${C_HTTP_PROJECT_DIR}/src/stx/stx_msg.c
#		${C_HTTP_PROJECT_DIR}/src/stx/stx_msg_parser.h
#		${C_HTTP_PROJECT_DIR}/src/stx/stx_msg_parser.c

		${C_HTTP_PROJECT_DIR}/apps/http_common/http_make_request_response.c
		${C_HTTP_PROJECT_DIR}/apps/http_common/http_make_request_response.h

		${C_HTTP_PROJECT_DIR}/apps/tcp_msg_stream/msg_stream.c
		${C_HTTP_PROJECT_DIR}/apps/tcp_msg_stream/msg_reader.c
		${C_HTTP_PROJECT_DIR}/apps/tcp_msg_stream/msg_writer.c

		${C_HTTP_PROJECT_DIR}/apps/sync_msg_stream/sync_msg_stream.c
		${C_HTTP_PROJECT_DIR}/apps/sync_msg_stream/sync_msg_stream.h
)

set(TEST_INCLUDE_DIRS
		${C_HTTP_PROJECT_DIR}/tests/test_runloop/test_echo
		${C_HTTP_PROJECT_DIR}/src
		${C_HTTP_PROJECT_DIR}
)
# this is a custom list of include dirs to ensure that this test does not include code
# outside the test dir other than rbl and common
set(INC_DIRS
		${C_HTTP_PROJECT_DIR}
		${C_HTTP_PROJECT_DIR}/src
		${C_HTTP_PROJECT_DIR}/tests/test_runloop/test_client_server_original
		${CMAKE_CURRENT_SOURCE_DIR}
)
function(mktarget name msgselect)
	set(target ${name}_${msgselect})
	set(MSG_SELECT_VAR "")
	string(TOUPPER ${msgselect} MSG_SELECT_VAR)
	set(MSG_SELECT_OPT MSG_SELECT_${MSG_SELECT_VAR})
	message("ZZZZZZZZZ")
	message("message select option ${MSG_SELECT_OPT}")
	message("target: ${target}")
	message("test client server original INC_DIRS ${INC_DIRS}")
	message("ZZZZZZZZZ")
	add_executable(${target} ${TARGET_SRC})
	if(msgselect STREQUAL "newline")
		target_compile_definitions(${target} PUBLIC MSG_SELECT_NEWLINE)
	elseif (msgselect STREQUAL "stx")
		target_compile_definitions(${target} PUBLIC MSG_SELECT_STX)
	elseif (msgselect STREQUAL "http")
		target_compile_definitions(${target} PUBLIC MSG_SELECT_HTTP)
	else()
		message( FATAL_ERROR "Msg selection not selected")
	endif()
 	target_include_directories(${target} PUBLIC ${INC_DIRS})
 	target_link_libraries(${target}	PUBLIC ${TEST_LINK_LIBS})
 	add_test(${target} ${target})
endfunction()

mktarget(test_client_server newline)
mktarget(test_client_server stx)
mktarget(test_client_server http)