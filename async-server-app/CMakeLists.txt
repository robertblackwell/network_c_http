add_executable(async-server-app
	async-server-main.c
)

target_include_directories(async-server-app
	PUBLIC SYSTEM ${C_HTTP_INCLUDE_PATHS})

target_link_libraries(async-server-app PUBLIC ${C_HTTP_LINK_LIBRARIES})



#add_executable(demo_verify_app
#		demo_verify.c
#		)
#target_include_directories(demo_verify_app
#		PUBLIC SYSTEM ${C_HTTP_INCLUDE_PATHS})
#
#target_link_libraries(demo_verify_app PUBLIC ${C_HTTP_LINK_LIBRARIES})
#
