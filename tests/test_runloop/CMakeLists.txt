# CHECK WE HAVE ALL THE VARIABLES WE NEED

message("test_include_dirs     ${TEST_INCLUDE_DIRS}")
message("test_link_libs        ${TEST_LINK_LIBS}")

# add_subdirectory(test_eventfd)
# add_subdirectory(test_q)
# add_subdirectory(test_q_asio)
# add_subdirectory(test_listener)
add_subdirectory(test_timers)
# add_subdirectory(test_io)
#add_subdirectory(test_io_eagain)
