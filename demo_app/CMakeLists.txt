add_executable(demo_app
	demo_main.c
	demo_process_main.c
)

target_include_directories(demo_app
	PUBLIC SYSTEM ${C_HTTP_INCLUDE_PATHS})

target_link_libraries(demo_app PUBLIC ${C_HTTP_LINK_LIBRARIES})



add_executable(demo_verify_app
		demo_verify.c
		)
target_include_directories(demo_verify_app
		PUBLIC SYSTEM ${C_HTTP_INCLUDE_PATHS})

target_link_libraries(demo_verify_app PUBLIC ${C_HTTP_LINK_LIBRARIES})

