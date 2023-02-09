add_executable(async_server_app
        async_server_app.c
)

target_include_directories(async_server_app
	PUBLIC SYSTEM ${C_HTTP_INCLUDE_PATHS})

target_link_libraries(async_server_app PUBLIC ${C_HTTP_LINK_LIBRARIES})

