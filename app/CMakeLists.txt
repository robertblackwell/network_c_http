add_executable(simple_server
	app.c
)

target_include_directories(simple_server
	PUBLIC SYSTEM ${C_EG_INCLUDE_PATHS})

target_link_libraries(simple_server PUBLIC ${C_EG_LINK_LIBRARIES})

