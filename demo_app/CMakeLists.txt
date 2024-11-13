add_executable(demo_async_app
		demo_async_app/demo_main.c
		demo_async_app/demo_process_main.c
)
target_include_directories(demo_async_app PUBLIC SYSTEM ${C_HTTP_INCLUDE_PATHS})
target_link_libraries(demo_async_app PUBLIC ${C_HTTP_LINK_LIBRARIES})

add_executable(demo_sync_app
		demo_sync_app/main.c
		demo_sync_app/process_main.c
)
target_include_directories(demo_sync_app PUBLIC SYSTEM ${C_HTTP_INCLUDE_PATHS})
target_link_libraries(demo_sync_app PUBLIC ${C_HTTP_LINK_LIBRARIES})


add_executable(demo_verify_app
		demo_verify_app/demo_verify.c
		demo_verify_app/verify_getopt.c
		demo_verify_app/verify_getopt.h
		demo_verify_app/verify_statistics.c
		demo_verify_app/verify_statistics.h
		demo_verify_app/verify_thread_context.c
		demo_verify_app/verify_thread_context.h
		)
target_include_directories(demo_verify_app PUBLIC SYSTEM ${C_HTTP_INCLUDE_PATHS})
target_link_libraries(demo_verify_app PUBLIC ${C_HTTP_LINK_LIBRARIES})

