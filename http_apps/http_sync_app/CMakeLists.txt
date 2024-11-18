add_executable(http_sync_app
	main.c
	process_main.c
)
target_include_directories(http_sync_app PUBLIC SYSTEM ${C_HTTP_INCLUDE_PATHS})
target_link_libraries(http_sync_app PUBLIC ${C_HTTP_LINK_LIBRARIES})
