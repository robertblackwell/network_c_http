if(off)
add_executable(test_client client.c)
target_compile_definitions(test_client PUBLIC ASIO_LISTENER_DISABLED)
target_include_directories(test_client PUBLIC ${TEST_INCLUDE_dirS} .)
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

		${C_HTTP_PROJECT_DIR}/apps/msg/newline_msg.h
		${C_HTTP_PROJECT_DIR}/apps/msg/msg_selection_header.h
		${C_HTTP_PROJECT_DIR}/apps/msg/newline_msg.c
		${C_HTTP_PROJECT_DIR}/apps/msg/newline_msg_parser.c
)
target_compile_definitions(test_echo_server PUBLIC ASIO_LISTENER_DISABLED)
target_include_directories(test_echo_server PUBLIC ${TEST_INCLUDE_dirS} .)
target_link_libraries(test_echo_server	PUBLIC ${TEST_LINK_LIBS})
#add_test(test_echo_server test_echo_server)
endif()
 add_executable(test_echo
        test_echo_v2.c
        echo_app/echo_app.h
        echo_app/echo_app.c

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

	${C_HTTP_PROJECT_DIR}/apps/sync_msg_stream/sync_msg_stream.h
	${C_HTTP_PROJECT_DIR}/apps/sync_msg_stream/sync_msg_stream.c

	${C_HTTP_PROJECT_DIR}/apps/msg/newline_msg.h
	${C_HTTP_PROJECT_DIR}/apps/msg/msg_selection_header.h
	${C_HTTP_PROJECT_DIR}/apps/msg/newline_msg.c
	${C_HTTP_PROJECT_DIR}/apps/msg/newline_msg_parser.c
 )
 target_compile_definitions(test_echo PUBLIC MSG_SELECT_ECHO)
 target_include_directories(test_echo PUBLIC ${TEST_INCLUDE_dirS} .)
 target_link_libraries(test_echo	PUBLIC ${TEST_LINK_LIBS})
 add_test(test_echo test_echo)
