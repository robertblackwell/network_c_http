# CHECK WE HAVE ALL THE VARIABLES WE NEED

message("WWWWWtest_include_dirs     ${TEST_INCLUDE_DIRS}")
message("WWWWWtest_link_libs        ${TEST_LINK_LIBS}")

set(TARGET test_mblock)
add_executable(${TARGET}
		test_mblock.c
		)

target_include_directories(${TARGET}
		PUBLIC
		${TEST_INCLUDE_DIRS})

target_link_libraries(${TARGET}
		PUBLIC ${TEST_LINK_LIBS})

add_test(${TARGET} ${TARGET})


