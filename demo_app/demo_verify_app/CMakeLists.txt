add_executable(demo_verify_app
		demo_verify.c
		verify_getopt.c
		verify_getopt.h
		verify_thread_context.c
		verify_thread_context.h
		../demo_common/demo_make-request_response.h
		../demo_common/demo_make_request_response.c
		)
target_include_directories(demo_verify_app PUBLIC SYSTEM ${C_HTTP_INCLUDE_PATHS})
target_link_libraries(demo_verify_app PUBLIC ${C_HTTP_LINK_LIBRARIES})

