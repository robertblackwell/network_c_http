set(SERVER_APP_SRC
    server_apps/echo_app.h
    server_apps/echo_app.c
    server/async_main.c
    server/async_process_main.c
    server/async_process_main.h
    server/server_ctx.h
    server/server_ctx.c

    tcp/tcp_stream.h
    tcp/tcp_stream_internal.h
    tcp/tcp_stream.c
    tcp/tcp_reader.c
    tcp/tcp_writer.c
    tcp/tcp_listener.c
    async_msg_stream/msg_stream.h
    async_msg_stream/msg_stream.c
    async_msg_stream/msg_reader.c
    async_msg_stream/msg_writer.c
    msg/newline_msg.h
    msg/newline_msg.c
    msg/newline_msg_parser.c
    msg/demo_msg.h
    msg/demo_msg.c
    msg/demo_msg_parser.c
)
set(verify_src
        sync_msg_stream/sync_msg_stream.c
        sync_msg_stream/sync_msg_stream.h

        msg/newline_msg.h
        msg/newline_msg.c
        msg/newline_msg_parser.c
        msg/demo_msg.h
        msg/demo_msg.c
        msg/demo_msg_parser.c

        client/verify_getopt.c
        client/verify_getopt.h
        client/verify_main.c
        client/verify_thread_context.c
        client/verify_thread_context.h
        client/verify_statistics.c
        client/verify_statistics.h

        client_verify_apps/verify_app.c
)
function(my_function arg_name1 arg_name2)
    # Function body
    message("Argument 1: ${arg_name1}")
    message("Argument 2: ${arg_name2}")
endfunction()

my_function("value 1" "value 2" "value 3")

function(build_server NAME APP_SELECTION MSG_SELECTION)
    set(TARGET ${NAME}_server)
    add_executable(${TARGET} ${SERVER_APP_SRC})
    set(C_HTTP_APPS_DIR ${C_HTTP_PROJECT_DIR}/apps)
    message("C_HTTP_SERVERS_DIR  ${C_HTTP_SERVERS_DIR}")
    get_target_property(defs ${TARGET} COMPILE_DEFINITIONS)
    list(FILTER defs EXCLUDE REGEX "APP_SELECT.*")
    target_compile_definitions(${TARGET} PUBLIC ${APP_SELECTION} ${MSG_SELECTION})
    target_include_directories(${TARGET} PUBLIC ${C_HTTP_INCLUDE_PATHS} ${C_HTTP_PROJECT_DIR}/apps})
    target_link_libraries(${TARGET} PUBLIC ${C_HTTP_LINK_LIBRARIES})
endfunction()
function(build_client NAME APP_SELECTION MSG_SELECTION)
    set(TARGET ${NAME}_verify)
    add_executable(${TARGET} ${verify_src})
    set(C_HTTP_APPS_DIR ${C_HTTP_PROJECT_DIR}/apps)
    message("C_HTTP_SERVERS_DIR  ${C_HTTP_SERVERS_DIR}")
    get_target_property(defs ${TARGET} COMPILE_DEFINITIONS)
    list(FILTER defs EXCLUDE REGEX "APP_SELECT.*")
    target_compile_definitions(${TARGET} PUBLIC ${APP_SELECTION} ${MSG_SELECTION})
    target_include_directories(${TARGET} PUBLIC ${C_HTTP_INCLUDE_PATHS} ${C_HTTP_PROJECT_DIR}/apps})
    target_link_libraries(${TARGET} PUBLIC ${C_HTTP_LINK_LIBRARIES})
endfunction()
build_server("echo_newline" "APP_SELECT_ECHO" "MSG_SELECT_ECHO")
build_server("echo_demo" "APP_SELECT_ECHO" "MSG_SELECT_DEMO")
build_client("echo_newline" "APP_SELECT_ECHO" "MSG_SELECT_ECHO")
build_client("echo_demo" "APP_SELECT_ECHO" "MSG_SELECT_DEMO")


