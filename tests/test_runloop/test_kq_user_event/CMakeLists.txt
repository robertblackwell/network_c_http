# CHECK WE HAVE ALL THE VARIABLES WE NEED

message("test_include_dirs     ${TEST_INCLUDE_DIRS}")
message("test_link_libs        ${TEST_LINK_LIBS}")

# test eventfd watcher
add_executable(test_user_event test_user_event.c)
target_compile_definitions(test_user_event PUBLIC RBL_LOG_ENABLE)
target_compile_definitions(test_user_event PUBLIC RBL_LOG_ALLOW_GLOBAL)

target_compile_definitions(test_user_event PUBLIC RUNLOOP_EVENTFD_TWO_PIPE_TRICK)
target_compile_definitions(test_user_event PUBLIC RUNLOOP_EVENTFD_SEMAPHORE)

target_include_directories(test_user_event	PUBLIC ${TEST_INCLUDE_DIRS})
target_link_libraries(test_user_event PUBLIC ${TEST_LINK_LIBS})
add_test(test_user_event test_user_event)

