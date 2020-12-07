# CHECK WE HAVE ALL THE VARIABLES WE NEED

message("test_include_dirs     ${TEST_INCLUDE_DIRS}")
message("test_link_libs        ${TEST_LINK_LIBS}")

#
# test timer watcher
#
set(TARGET_TIMER test_timer)
add_executable(${TARGET_TIMER}
	test_timer.c
)

target_include_directories(${TARGET_TIMER}
	PUBLIC 
		${TEST_INCLUDE_dirS})

target_link_libraries(${TARGET_TIMER}
	PUBLIC ${TEST_LINK_LIBS})

add_test(${TARGET_TIMER} ${TARGET_TIMER})
#
# test timer disarm
#
set(TARGET_DISARM test_disarm)
add_executable(${TARGET_DISARM}
		test_timer_disarm.c
		)

target_include_directories(${TARGET_DISARM}
		PUBLIC
		${TEST_INCLUDE_dirS})

target_link_libraries(${TARGET_DISARM}
		PUBLIC ${TEST_LINK_LIBS})

add_test(${TARGET_DISARM} ${TARGET_DISARM})

#
# test socket io watcher
#

set(TARGET_IO test_io)
add_executable(${TARGET_IO}
		test_io.c
		io_read.h
		io_read.c
		io_write.h
		io_write.c
		)

target_include_directories(${TARGET_IO}
		PUBLIC
		${TEST_INCLUDE_dirS})

target_link_libraries(${TARGET_IO}
		PUBLIC ${TEST_LINK_LIBS})

add_test(${TARGET_IO} ${TARGET_IO})

#
# test queue watcher
#

set(TARGET_Q test_q)
add_executable(${TARGET_Q}
		test_q.c
		)

target_include_directories(${TARGET_Q}
		PUBLIC
		${TEST_INCLUDE_dirS})

target_link_libraries(${TARGET_Q}
		PUBLIC ${TEST_LINK_LIBS})

#add_test(${TARGET_IO} ${TARGET_Q})
