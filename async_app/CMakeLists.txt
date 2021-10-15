add_executable(async_server
	async_server.c
)

target_include_directories(async_server
	PUBLIC SYSTEM ${C_HTTP_INCLUDE_PATHS})

target_link_libraries(async_server PUBLIC ${C_HTTP_LINK_LIBRARIES})

