add_executable(demo_verify_app
        demo_verify_main.c
        demo_verify_getopt.c
        demo_verify_getopt.h
        demo_verify_thread_context.c
        demo_verify_thread_context.h
        ../demo_common/demo_make_request_response.h
        ../demo_common/demo_make_request_response.c
		)
target_include_directories(demo_verify_app PUBLIC SYSTEM ${C_HTTP_INCLUDE_PATHS}  ${APP_INCLUDE_DIR})
target_link_libraries(demo_verify_app PUBLIC ${C_HTTP_LINK_LIBRARIES})

