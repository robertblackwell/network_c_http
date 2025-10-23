# CHECK WE HAVE ALL THE VARIABLES WE NEED

message("test_include_dirs     ${TEST_INCLUDE_DIRS}")
message("test_link_libs        ${TEST_LINK_LIBS}")

set(TARGET test_coredump)
add_executable(${TARGET}
	test_coredump.c
)


add_test(${TARGET} ${TARGET})


