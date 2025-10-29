
#set(test_listener_src
#		test_listener.c
#		listener_ctx.h
#		listener_ctx.c
#		connector_ctx.h
#		connector_ctx.c
#)
#set(test_listener_asio_src
#		test_listener.c
#		listener_ctx.h
#		asio_listener_ctx.c
#		connector_ctx.h
#		connector_ctx.c
#)
#
#add_executable(test_client client.c)
#target_compile_definitions(test_client PUBLIC ASIO_LISTENER_DISABLED)
#target_include_directories(test_client PUBLIC ${TEST_INCLUDE_dirS} ${CMAKE_CURRENT_SOURCE_DIR})
#target_link_libraries(test_client	PUBLIC ${TEST_LINK_LIBS})
##add_test(test_client test_client)
#
#add_executable(test_echo_server
#		main.c
#		simple_request_response_app/simple_request_response_app.h
#		simple_request_response_app/simple_request_response_app.c
#
#		server/server_ctx.h
#		server/server_ctx.c
#
#		tcp/tcp_stream.h
#		tcp/tcp_stream_internal.h
#		tcp/tcp_stream.c
#		tcp/tcp_reader.c
#		tcp/tcp_writer.c
#		tcp/tcp_listener.c
#
#		msg/msg_stream.h
#		msg/msg_stream.c
#		msg/msg_reader.c
#		msg/msg_writer.c
#		msg/msg_parser.c
#)
#target_compile_definitions(test_echo_server PUBLIC ASIO_LISTENER_DISABLED)
#target_include_directories(test_echo_server PUBLIC ${TEST_INCLUDE_dirS} ${CMAKE_CURRENT_SOURCE_DIR})
#target_link_libraries(test_echo_server	PUBLIC ${TEST_LINK_LIBS})
##add_test(test_echo_server test_echo_server)

set(target test_client_server_original)
set(TARGET_SRC
		test_client_server.c
		simple_request_response_app/simple_app.h
		simple_request_response_app/simple_app.c

	server/server_ctx.h
	server/server_ctx.c

	tcp/tcp_stream.h
	tcp/tcp_stream_internal.h
	tcp/tcp_stream.c
	tcp/tcp_reader.c
	tcp/tcp_writer.c
	tcp/tcp_listener.c

	msg/msg_generic.h
	msg/msg_generic.c

	msg/newline/newline_msg.h
	msg/newline/newline_msg.c
	msg/newline/newline_msg_parser.c

	msg/stx/stx_msg.h
	msg/stx/stx_msg.c
	msg/stx/stx_msg_parser.h
	msg/stx/stx_msg_parser.c

	mstream/msg_stream.c
	mstream/msg_reader.c
	mstream/msg_writer.c

	sync_msg_stream/sync_msg_stream.c
	sync_msg_stream/sync_msg_stream.h
)

#add_executable(${target} ${TARGET_SRC})
#	test_echo.c
#	simple_request_response_app/simple_request_response_app.h
#	simple_request_response_app/simple_request_response_app.c
#
# 	server/server_ctx.h
#	server/server_ctx.c
#
#	tcp/tcp_stream.h
#	tcp/tcp_stream_internal.h
#	tcp/tcp_stream.c
#	tcp/tcp_reader.c
#	tcp/tcp_writer.c
#	tcp/tcp_listener.c
#
#		msg/msg_generic.h
#		msg/msg_generic.c
#
#		msg/newline/newline_msg.h
#		msg/newline/newline_msg.c
#		msg/newline/newline_msg_parser.c
#
#		msg/stx/stx_msg.h
#		msg/stx/stx_msg.c
#		msg/stx/stx_msg_parser.h
#		msg/stx/stx_msg_parser.c
#
#		mstream/msg_stream.c
#		mstream/msg_reader.c
#		mstream/msg_writer.c
#
#		sync_msg_stream/sync_msg_stream.c
#		sync_msg_stream/sync_msg_stream.h
# )
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
	else()
		message( FATAL_ERROR "Msg selection not selected")
	endif()
 	target_include_directories(${target} PUBLIC ${INC_DIRS})
 	target_link_libraries(${target}	PUBLIC ${TEST_LINK_LIBS})
 	add_test(${target} ${target})
endfunction()

mktarget(test_client_server_original newline)
mktarget(test_client_server_original stx)