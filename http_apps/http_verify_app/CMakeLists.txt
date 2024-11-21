add_executable(http_verify_app
	http_verify.c
		../http_common/http_make_request_response.c
		../http_common/http_make_request_response.h
	verify_getopt.c
	verify_getopt.h
        ../../http_in_c/common/verify_statistics.c
        ../../http_in_c/common/verify_statistics.h
	verify_thread_context.c
	verify_thread_context.h
		)
target_include_directories(http_verify_app PUBLIC SYSTEM ${C_HTTP_INCLUDE_PATHS})
target_link_libraries(http_verify_app PUBLIC ${C_HTTP_LINK_LIBRARIES})

