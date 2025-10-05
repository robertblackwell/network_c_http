# CHECK WE HAVE ALL THE VARIABLES WE NEED

message("test_include_dirs     ${TEST_INCLUDE_DIRS}")
message("test_link_libs        ${TEST_LINK_LIBS}")
# test_q_asio
add_executable(test_q_asio test_q_asio.c )
target_include_directories(test_q_asio PUBLIC ${TEST_INCLUDE_dirS})
target_link_libraries(test_q_asio PUBLIC ${TEST_LINK_LIBS})
add_test(test_q_asio test_q_asio)

