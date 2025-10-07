add_executable(test_io
		test_io.c
		io_read.h
		io_read.c
		io_write.h
		io_write.c
		)

target_include_directories(test_io PUBLIC ${TEST_INCLUDE_dirS})
target_link_libraries(test_io PUBLIC ${TEST_LINK_LIBS})
add_test(test_io test_io)

