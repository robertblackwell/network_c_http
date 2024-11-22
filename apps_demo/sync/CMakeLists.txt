add_executable(demo_sync_app
        demo_sync_main.c
		demo_sync_process_main.c
		../demo_common/demo_make_request_response.h
		../demo_common/demo_make_request_response.c
)
target_include_directories(demo_sync_app PUBLIC SYSTEM ${C_HTTP_INCLUDE_PATHS}  ${APP_INCLUDE_DIR})
target_link_libraries(demo_sync_app PUBLIC ${C_HTTP_LINK_LIBRARIES})
