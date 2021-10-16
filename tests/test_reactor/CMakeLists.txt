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
# test fdevent watcher
#

set(TARGET_FDEV test_fdevent)
add_executable(${TARGET_FDEV}
		test_fdevent.c
		)

target_include_directories(${TARGET_FDEV}
		PUBLIC
		${TEST_INCLUDE_dirS})

target_link_libraries(${TARGET_FDEV}
		PUBLIC ${TEST_LINK_LIBS})

add_test(${TARGET_FDEV} ${TARGET_FDEV})

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

add_subdirectory(test_listener)
#
# test listener
#
#
#set(TARGET_LISTENER test_listener)
#add_executable(${TARGET_LISTENER}
#		test_listener.c
#		listener.h
#		listener.c
#		connector.h
#		connector.c
#		)
#
#target_include_directories(${TARGET_LISTENER}
#		PUBLIC
#		${TEST_INCLUDE_dirS})
#
#target_link_libraries(${TARGET_LISTENER}
#		PUBLIC ${TEST_LINK_LIBS})
#
#add_test(${TARGET_LISTENER} ${TARGET_LISTENER})
