# CHECK WE HAVE ALL THE VARIABLES WE NEED

message("apple_test_include_dirs     ${APPLE_TEST_INCLUDE_DIRS}")
message("apple_test_link_libs        ${APPLE_TEST_LINK_LIBS}")

set(TARGET apple_test_buffers)
add_executable(${TARGET}
		test_buffer_main.c
		)

target_include_directories(${TARGET}
		PUBLIC
		${APPLE_TEST_INCLUDE_DIRS})

target_link_libraries(${TARGET}
		PUBLIC ${APPLE_TEST_LINK_LIBS})

add_test(${TARGET} ${TARGET})


