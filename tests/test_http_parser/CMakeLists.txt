# CHECK WE HAVE ALL THE VARIABLES WE NEED

message("test_include_dirs     ${TEST_INCLUDE_DIRS}")
message("test_link_libs        ${TEST_LINK_LIBS}")

set(TARGET test_http_parser)
add_executable(${TARGET}
		test_main.c
		run_list.c
		test_harness.c
		test_harness.h
		helper.h
		test_suite_requests.c
		test_suite_responses.c
		)

target_include_directories(${TARGET}
		PUBLIC
		${TEST_INCLUDE_dirS})

target_link_libraries(${TARGET}
		PUBLIC ${TEST_LINK_LIBS})

add_test(${TARGET} ${TARGET})


