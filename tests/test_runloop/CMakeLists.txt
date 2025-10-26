# CHECK WE HAVE ALL THE VARIABLES WE NEED

message("test_include_dirs     ${TEST_INCLUDE_DIRS}")
message("test_link_libs        ${TEST_LINK_LIBS}")

# test eventfd watcher
add_executable(test_eventfd test_eventfd.c)
target_compile_definitions(test_eventfd PUBLIC RBL_LOG_ENABLE)
target_compile_definitions(test_eventfd PUBLIC RBL_LOG_ALLOW_GLOBAL)

target_compile_definitions(test_eventfd PUBLIC RUNLOOP_EVENTFD_TWO_PIPE_TRICK)
target_compile_definitions(test_eventfd PUBLIC RUNLOOP_EVENTFD_SEMAPHORE)

target_include_directories(test_eventfd	PUBLIC ${TEST_INCLUDE_DIRS})
target_link_libraries(test_eventfd PUBLIC ${TEST_LINK_LIBS})
add_test(test_eventfd test_eventfd)

# test queue watcher
add_executable(test_q	test_q.c)
#target_compile_definitions(${TARGET_Q} PUBLIC RBL_TAG_CHECK_ON=1)
target_include_directories(test_q PUBLIC ${TEST_INCLUDE_DIRS})
target_link_libraries(test_q PUBLIC ${TEST_LINK_LIBS})
add_test(test_q test_q)

if(0)
# test_q_asio
add_executable(test_q_asio test_q_asio.c )
target_include_directories(test_q_asio PUBLIC ${TEST_INCLUDE_dirS})
target_link_libraries(test_q_asio PUBLIC ${TEST_LINK_LIBS})
add_test(test_q_asio test_q_asio)
endif()

add_subdirectory(test_listener)
add_subdirectory(test_timers)
add_subdirectory(test_io)
add_subdirectory(test_client_server_original)
