# CHECK WE HAVE ALL THE VARIABLES WE NEED

message("test_include_dirs     ${TEST_INCLUDE_DIRS}")
message("test_link_libs        ${TEST_LINK_LIBS}")

set(TARGET test_stx_msg_parser)
add_executable(${TARGET}
        test_stx_msg_parser.c
        test_harness.c
        test_harness.h
		../../src/stx/stx_msg.c
		../../src/stx/stx_msg.h
		../../src/stx/stx_msg_parser.c
		../../src/stx/stx_msg_parser.h

)

target_include_directories(${TARGET}
		PUBLIC
		${TEST_INCLUDE_dirS})

target_link_libraries(${TARGET}
		PUBLIC ${TEST_LINK_LIBS})

add_test(${TARGET} ${TARGET})


