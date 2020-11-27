add_executable(xr_server
	app.c
)

target_include_directories(xr_server
	PUBLIC SYSTEM ${C_HTTP_INCLUDE_PATHS})

target_link_libraries(xr_server PUBLIC ${C_HTTP_LINK_LIBRARIES})

