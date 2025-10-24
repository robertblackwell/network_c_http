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

if(0)
add_executable(test_io_asio
		test_io_asio.c
        io_read_asio.h
        io_read_asio.c
		io_write_asio.h
		io_write_asio.c
)

target_include_directories(test_io_asio PUBLIC ${TEST_INCLUDE_dirS})
target_link_libraries(test_io_asio PUBLIC ${TEST_LINK_LIBS})
add_test(test_io_asio test_io_asio)
endif()