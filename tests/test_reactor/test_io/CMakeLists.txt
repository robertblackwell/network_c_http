
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

