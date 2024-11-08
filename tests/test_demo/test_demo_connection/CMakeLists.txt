add_executable(test_fill_iobuffer
		test_fill_iobuffer.c
		fill_iobuffer.h
		fill_iobuffer.c
		)

target_include_directories(test_fill_iobuffer PUBLIC ${TEST_INCLUDE_dirS})
target_link_libraries(test_fill_iobuffer PUBLIC ${TEST_LINK_LIBS})
add_test(test_fill_iobuffer test_fill_iobuffer)

add_executable(test_demo_io
	test_demo_io.c
	fill_iobuffer.h
	fill_iobuffer.c
	io_read_asio.h
	io_read_asio.c
	demo_write.h
	demo_write.c
)
#
target_include_directories(test_demo_io PUBLIC ${TEST_INCLUDE_dirS})
target_link_libraries(test_demo_io PUBLIC ${TEST_LINK_LIBS})
add_test(test_demo_io test_demo_io)

add_executable(test_demo_full
	test_demo_full.c
	fill_iobuffer.h
	fill_iobuffer.c
	demo_read.h
	demo_read.c
	demo_write.h
	demo_write.c
)
#
target_include_directories(test_demo_full PUBLIC ${TEST_INCLUDE_dirS})
target_link_libraries(test_demo_full PUBLIC ${TEST_LINK_LIBS})
add_test(test_demo_full test_demo_full)
