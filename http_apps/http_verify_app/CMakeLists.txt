add_executable(http_verify_app
	http_verify.c
	verify_getopt.c
	verify_getopt.h
	verify_statistics.c
	verify_statistics.h
	verify_thread_context.c
	verify_thread_context.h
		)
target_include_directories(http_verify_app PUBLIC SYSTEM ${C_HTTP_INCLUDE_PATHS})
target_link_libraries(http_verify_app PUBLIC ${C_HTTP_LINK_LIBRARIES})

