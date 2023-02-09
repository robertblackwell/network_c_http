add_executable(sync_server_app
        sync_server_app.c
)

target_include_directories(sync_server_app
	PUBLIC SYSTEM ${C_HTTP_INCLUDE_PATHS})

target_link_libraries(sync_server_app PUBLIC ${C_HTTP_LINK_LIBRARIES})

