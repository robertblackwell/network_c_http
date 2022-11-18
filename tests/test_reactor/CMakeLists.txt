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
set(TARGET_TIMER_DISARM test_timer_disarm)
add_executable(${TARGET_TIMER_DISARM}
		test_timer_disarm.c
		)

target_include_directories(${TARGET_TIMER_DISARM}
		PUBLIC
		${TEST_INCLUDE_dirS})

target_link_libraries(${TARGET_TIMER_DISARM}
		PUBLIC ${TEST_LINK_LIBS})

add_test(${TARGET_TIMER_DISARM} ${TARGET_TIMER_DISARM})


#
# test fdevent watcher
#
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

add_test(${TARGET_Q} ${TARGET_Q})

#
# test interthread queue
#

set(TARGET_ITQ test_itq)
add_executable(${TARGET_ITQ}
		test_itq.c
		)

target_include_directories(${TARGET_ITQ}
		PUBLIC
		${TEST_INCLUDE_dirS})

target_link_libraries(${TARGET_ITQ}
		PUBLIC ${TEST_LINK_LIBS})

add_test(${TARGET_ITQ} ${TARGET_ITQ})


#add_subdirectory(test_io)
#add_subdirectory(test_listener)
