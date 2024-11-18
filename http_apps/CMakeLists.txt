# add_executable(http_async_app
# 		http_async_app/http_main.c
# 		http_async_app/http_process_main.c
# )
# target_include_directories(http_async_app PUBLIC SYSTEM ${C_HTTP_INCLUDE_PATHS})
# target_link_libraries(http_async_app PUBLIC ${C_HTTP_LINK_LIBRARIES})

# add_executable(http_sync_app
# 		http_sync_app/main.c
# 		http_sync_app/process_main.c
# )
# target_include_directories(http_sync_app PUBLIC SYSTEM ${C_HTTP_INCLUDE_PATHS})
# target_link_libraries(http_sync_app PUBLIC ${C_HTTP_LINK_LIBRARIES})


# add_executable(http_verify_app
# 		http_verify_app/http_verify.c
# 		http_verify_app/verify_getopt.c
# 		http_verify_app/verify_getopt.h
# 		http_verify_app/verify_statistics.c
# 		http_verify_app/verify_statistics.h
# 		http_verify_app/verify_thread_context.c
# 		http_verify_app/verify_thread_context.h
# 		)
# target_include_directories(http_verify_app PUBLIC SYSTEM ${C_HTTP_INCLUDE_PATHS})
# target_link_libraries(http_verify_app PUBLIC ${C_HTTP_LINK_LIBRARIES})

add_subdirectory(http_async_app)
add_subdirectory(http_sync_app)
add_subdirectory(http_verify_app)