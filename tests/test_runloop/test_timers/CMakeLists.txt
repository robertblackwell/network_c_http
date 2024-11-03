# CHECK WE HAVE ALL THE VARIABLES WE NEED

message("test_include_dirs     ${TEST_INCLUDE_DIRS}")
message("test_link_libs        ${TEST_LINK_LIBS}")

#
# test timer_non_repeating
#
add_executable(test_timer_non_repeating test_timer_non_repeating.c)
target_include_directories(test_timer_non_repeating PUBLIC ${TEST_INCLUDE_dirS})
target_link_libraries(test_timer_non_repeating PUBLIC ${TEST_LINK_LIBS})
add_test(test_timer_non_repeating test_timer_non_repeating)

#
# test timer_single_repeating
#
add_executable(test_timer_single_repeating test_timer_single_repeating.c)
target_include_directories(test_timer_single_repeating PUBLIC ${TEST_INCLUDE_dirS})
target_link_libraries(test_timer_single_repeating PUBLIC ${TEST_LINK_LIBS})
add_test(test_timer_single_repeating test_timer_single_repeating)


#
# test timer_post
#
add_executable(test_timer_multiple_repeating test_timer_multiple_repeating.c)
target_include_directories(test_timer_multiple_repeating PUBLIC ${TEST_INCLUDE_dirS})
target_link_libraries(test_timer_multiple_repeating PUBLIC ${TEST_LINK_LIBS})
add_test(test_timer_multiple_repeating test_timer_multiple_repeating)

#
# test timer_post
#
add_executable(test_timer_post test_timer_post.c)
target_include_directories(test_timer_post PUBLIC ${TEST_INCLUDE_dirS})
target_link_libraries(test_timer_post PUBLIC ${TEST_LINK_LIBS})
add_test(test_timer_post test_timer_post)

#
# test timer_multithreaded
#
#add_executable(test_timer_multithread test_timer_multithread.c)
#target_include_directories(test_timer_multithread PUBLIC ${TEST_INCLUDE_dirS})
#target_link_libraries(test_timer_multithread PUBLIC ${TEST_LINK_LIBS})
#add_test(test_timer_multithread test_timer_multithread)
