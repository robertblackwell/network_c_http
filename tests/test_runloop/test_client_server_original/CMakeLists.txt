
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

set(target test_client_server_original)
add_executable(${target}
	test_echo.c
	echo_app/echo_app.h
	echo_app/echo_app.c

 	server/server_ctx.h
	server/server_ctx.c

	tcp/tcp_stream.h
	tcp/tcp_stream_internal.h
	tcp/tcp_stream.c
	tcp/tcp_reader.c
	tcp/tcp_writer.c
	tcp/tcp_listener.c

	msg/msg_stream.h
	msg/msg_stream.c
	msg/msg_reader.c
	msg/msg_writer.c
	msg/msg_parser.c
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
message("ZZZZZZZtest client server INC_DIRS ${INC_DIRS}")
# target_compile_definitions(test_echo PUBLIC ASIO_LISTENER_ENABLED)
 target_include_directories(${target} PUBLIC ${INC_DIRS})
 target_link_libraries(${target}	PUBLIC ${TEST_LINK_LIBS})
 add_test(${target} ${target})
