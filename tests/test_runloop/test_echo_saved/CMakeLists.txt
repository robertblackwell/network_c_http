
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
if(off)
add_executable(test_client client.c)
target_compile_definitions(test_client PUBLIC ASIO_LISTENER_DISABLED)
target_include_directories(test_client PUBLIC ${TEST_INCLUDE_dirS} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_client	PUBLIC ${TEST_LINK_LIBS})
#add_test(test_client test_client)

add_executable(test_echo_server
		main.c
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
		msg/message.h
		msg/message_internal.h
		msg/message.c
		msg/msg_parser.c
)
target_compile_definitions(test_echo_server PUBLIC ASIO_LISTENER_DISABLED)
target_include_directories(test_echo_server PUBLIC ${TEST_INCLUDE_dirS} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_echo_server	PUBLIC ${TEST_LINK_LIBS})
#add_test(test_echo_server test_echo_server)
endif()
message("TEST_ECHO test include dir [${TEST_INCLUDE_DIRS}]")
 add_executable(test_echo
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
		 msg/message.h
		 msg/message_internal.h
		 msg/message.c
		 msg/msg_parser.c
 )
 target_compile_definitions(test_echo PUBLIC ASIO_LISTENER_ENABLED)
 target_include_directories(test_echo PUBLIC ${TEST_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
 target_link_libraries(test_echo	PUBLIC ${TEST_LINK_LIBS})
 add_test(test_echo test_echo)
