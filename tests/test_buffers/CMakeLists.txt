# CHECK WE HAVE ALL THE VARIABLES WE NEED

message("WWWWWtest_include_dirs     ${TEST_INCLUDE_DIRS}")
message("WWWWWtest_link_libs        ${TEST_LINK_LIBS}")

set(CTARGET_M test_cbuffer_malloc)
add_executable(${CTARGET_M}
		test_cbuffer.c
)
target_compile_definitions(${CTARGET_M} PUBLIC CBUF_ALLOCATOR_MALLOC)
target_include_directories(${CTARGET_M}
		PUBLIC
		${TEST_INCLUDE_DIRS})
target_link_libraries(${CTARGET_M}
		PUBLIC ${TEST_LINK_LIBS})
add_test(${CTARGET_M} ${CTARGET_M})

set(CTARGET test_cbuffer_arena)
add_executable(${CTARGET}
		test_cbuffer.c
)
target_include_directories(${CTARGET}
		PUBLIC
		${TEST_INCLUDE_DIRS})
target_link_libraries(${CTARGET}
		PUBLIC ${TEST_LINK_LIBS})
add_test(${CTARGET} ${CTARGET})


set(TARGET test_buffers)
add_executable(${TARGET}
		test_buffer_main.c
		)

target_include_directories(${TARGET}
		PUBLIC
		${TEST_INCLUDE_DIRS})

target_link_libraries(${TARGET}
		PUBLIC ${TEST_LINK_LIBS})

add_test(${TARGET} ${TARGET})


