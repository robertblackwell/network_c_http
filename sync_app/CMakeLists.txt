add_executable(simple_server
		sync_app.c
)

target_include_directories(simple_server
	PUBLIC SYSTEM ${C_HTTP_INCLUDE_PATHS})

target_link_libraries(simple_server PUBLIC ${C_HTTP_LINK_LIBRARIES})

