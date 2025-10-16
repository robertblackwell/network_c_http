
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
#		echo_app/echo_app.h
#		echo_app/echo_app.c
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

set(TARGET test_echo_step_2)
add_executable(${TARGET}
	test_echo.c
	test_ctx.h
	test_ctx.c
	client/client_thread_function.c

	${C_HTTP_PROJECT_DIR}/apps/server_apps/echo_app.h
	${C_HTTP_PROJECT_DIR}/apps/server_apps/echo_app.c

 	server/server_ctx.h
	server/server_ctx.c

	${C_HTTP_PROJECT_DIR}/apps/tcp/tcp_stream.h
	${C_HTTP_PROJECT_DIR}/apps/tcp/tcp_stream_internal.h
	${C_HTTP_PROJECT_DIR}/apps/tcp/tcp_stream.c
	${C_HTTP_PROJECT_DIR}/apps/tcp/tcp_reader.c
	${C_HTTP_PROJECT_DIR}/apps/tcp/tcp_writer.c
	${C_HTTP_PROJECT_DIR}/apps/tcp/tcp_listener.c

		${C_HTTP_PROJECT_DIR}/apps/async_msg_stream/msg_stream.h
		${C_HTTP_PROJECT_DIR}/apps/async_msg_stream/msg_stream.c
		${C_HTTP_PROJECT_DIR}/apps/async_msg_stream/msg_reader.c
		${C_HTTP_PROJECT_DIR}/apps/async_msg_stream/msg_writer.c
		${C_HTTP_PROJECT_DIR}/apps/msg/newline_msg_parser.c
		${C_HTTP_PROJECT_DIR}/apps/msg/newline_msg.c
		${C_HTTP_PROJECT_DIR}/apps/msg/newline_msg.h
 )
set(TEST_INCLUDE_DIRS
		${C_HTTP_PROJECT_DIR}/tests/test_runloop/test_echo_step_2
		${C_HTTP_PROJECT_DIR}/apps
		${C_HTTP_PROJECT_DIR}/src
		${C_HTTP_PROJECT_DIR}
)
 target_compile_definitions(${TARGET} PUBLIC APP_SELECT_ECHO MSG_SELECT_ECHO)
 target_include_directories(${TARGET} PUBLIC ${TEST_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
 target_link_libraries(${TARGET}	PUBLIC ${TEST_LINK_LIBS})
 add_test(${TARGET} ${TARGET})
