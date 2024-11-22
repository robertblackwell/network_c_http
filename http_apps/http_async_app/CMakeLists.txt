add_executable(http_async_app
	http_main.c
	http_process_main.c
		../http_common/http_make_request_response.c
		../http_common/http_make_request_response.h

)
target_include_directories(http_async_app PUBLIC SYSTEM ${C_HTTP_INCLUDE_PATHS})
target_link_libraries(http_async_app PUBLIC ${C_HTTP_LINK_LIBRARIES})
