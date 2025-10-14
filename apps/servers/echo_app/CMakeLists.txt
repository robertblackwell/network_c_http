
set(TARGET echo_server)
add_executable(${TARGET}
		../async_main.c
		../async_process_main.c
		../async_process_main.h

	echo_app.h
	echo_app.c

		../server/server_ctx.h
		../server/server_ctx.c

		../tcp/tcp_stream.h
		../tcp/tcp_stream_internal.h
		../tcp/tcp_stream.c
		../tcp/tcp_reader.c
		../tcp/tcp_writer.c
		../tcp/tcp_listener.c

		../msg/msg_stream.h
		../msg/msg_stream.c
		../msg/msg_reader.c
		../msg/msg_writer.c
		../msg/message.h
		../msg/message_internal.h
		../msg/message.c
		../msg/msg_parser.c
 )
set(C_HTTP_SERVERS_DIR ${C_HTTP_PROJECT_DIR}/apps/servers)
message("C_HTTP_SERVERS_DIR  ${C_HTTP_SERVERS_DIR}")
 target_include_directories(${TARGET} PUBLIC ${C_HTTP_INCLUDE_PATHS} ${C_HTTP_PROJECT_DIR}/apps/servers})
 target_link_libraries(${TARGET} PUBLIC ${C_HTTP_LINK_LIBRARIES})
