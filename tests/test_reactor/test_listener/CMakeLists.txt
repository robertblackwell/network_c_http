
set(TARGET_LISTENER test_listener)
add_executable(${TARGET_LISTENER}
		test_listener.c
		listener.h
		listener.c
		connector.h
		connector.c
		)

target_include_directories(${TARGET_LISTENER}
		PUBLIC
		${TEST_INCLUDE_dirS})

target_link_libraries(${TARGET_LISTENER}
		PUBLIC ${TEST_LINK_LIBS})

add_test(${TARGET_LISTENER} ${TARGET_LISTENER})
