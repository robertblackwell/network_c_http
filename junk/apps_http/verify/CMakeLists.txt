add_executable(http_verify_app
        http_verify_main.c
        ../http_common/http_make_request_response.c
        ../http_common/http_make_request_response.h
        http_verify_getopt.c
        http_verify_getopt.h
		../../../apps/client/verify_statistics.c
		../../../apps/client/verify_statistics.h
        http_verify_thread_context.c
        http_verify_thread_context.h
		)
target_include_directories(http_verify_app PUBLIC SYSTEM ${C_HTTP_INCLUDE_PATHS}  ${APP_INCLUDE_DIR})
target_link_libraries(http_verify_app PUBLIC ${C_HTTP_LINK_LIBRARIES})

