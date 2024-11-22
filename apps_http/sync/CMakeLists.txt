add_executable(http_sync_app
        http_sync_main.c
		http_sync_process_main.c
		../http_common/http_make_request_response.c
		../http_common/http_make_request_response.h
)
target_include_directories(http_sync_app PUBLIC SYSTEM ${C_HTTP_INCLUDE_PATHS}  ${APP_INCLUDE_DIR})
target_link_libraries(http_sync_app PUBLIC ${C_HTTP_LINK_LIBRARIES})
