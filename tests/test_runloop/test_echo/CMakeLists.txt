
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

add_executable(test_client client.c)
target_compile_definitions(test_client PUBLIC ASIO_LISTENER_DISABLED)
target_include_directories(test_client PUBLIC ${TEST_INCLUDE_dirS} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_client	PUBLIC ${TEST_LINK_LIBS})
add_test(test_client test_client)

add_executable(test_server
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
		msg/msg_parser.c
)
target_compile_definitions(test_server PUBLIC ASIO_LISTENER_DISABLED)
target_include_directories(test_server PUBLIC ${TEST_INCLUDE_dirS} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_server	PUBLIC ${TEST_LINK_LIBS})
add_test(test_server test_server)

# add_executable(test_listener_asio
# 		test_listener_asio.c
# 		listener_ctx.h
# 		asio_listener_ctx.c
# 		connector_ctx.h
# 		connector_ctx.c
# )
# target_compile_definitions(test_listener_asio PUBLIC ASIO_LISTENER_ENABLED)
# target_include_directories(test_listener_asio PUBLIC ${TEST_INCLUDE_dirS})
# target_link_libraries(test_listener_asio	PUBLIC ${TEST_LINK_LIBS})
# add_test(test_listener_asio test_listener_asio)
