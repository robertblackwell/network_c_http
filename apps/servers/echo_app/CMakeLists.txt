
set(TARGET echo_server)
set(DEMO_TARGET demo_server)
set(SERVER_APP_SRC
		echo_app.h
		echo_app.c
		../async_main.c
		../async_process_main.c
		../async_process_main.h
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
		../msg/echo_msg.h
		../msg/echo_msg.c
		../msg/echo_msg_parser.c
		../msg/demo_msg.h
		../msg/demo_msg.c
		../msg/demo_msg_parser.c
)
add_executable(${TARGET} ${SERVER_APP_SRC})
set(C_HTTP_SERVERS_DIR ${C_HTTP_PROJECT_DIR}/apps/servers)
message("C_HTTP_SERVERS_DIR  ${C_HTTP_SERVERS_DIR}")
get_target_property(defs ${TARGET} COMPILE_DEFINITIONS)
list(FILTER defs EXCLUDE REGEX "APP_SELECT.*")
target_compile_definitions(${TARGET} PUBLIC APP_SELECT_ECHO MSG_SELECT_ECHO)
target_include_directories(${TARGET} PUBLIC ${C_HTTP_INCLUDE_PATHS} ${C_HTTP_PROJECT_DIR}/apps/servers})
target_link_libraries(${TARGET} PUBLIC ${C_HTTP_LINK_LIBRARIES})

add_executable(${DEMO_TARGET} ${SERVER_APP_SRC})
set(C_HTTP_SERVERS_DIR ${C_HTTP_PROJECT_DIR}/apps/servers)
message("C_HTTP_SERVERS_DIR  ${C_HTTP_SERVERS_DIR}")
get_target_property(defs ${DEMO_TARGET} COMPILE_DEFINITIONS)
list(FILTER defs EXCLUDE REGEX "APP_SELECT*")
target_compile_definitions(${DEMO_TARGET} PUBLIC APP_SELECT_DEMO MSG_SELECT_DEMO)
target_include_directories(${DEMO_TARGET} PUBLIC ${C_HTTP_INCLUDE_PATHS} ${C_HTTP_PROJECT_DIR}/apps/servers})
target_link_libraries(${DEMO_TARGET} PUBLIC ${C_HTTP_LINK_LIBRARIES})
