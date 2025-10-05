# CHECK WE HAVE ALL THE VARIABLES WE NEED

message("test_include_dirs     ${TEST_INCLUDE_DIRS}")
message("test_link_libs        ${TEST_LINK_LIBS}")

# test queue watcher
add_executable(test_q	test_q.c)
#target_compile_definitions(${TARGET_Q} PUBLIC RBL_TAG_CHECK_ON=1)
target_include_directories(test_q PUBLIC ${TEST_INCLUDE_DIRS})
target_link_libraries(test_q PUBLIC ${TEST_LINK_LIBS})
add_test(test_q test_q)

