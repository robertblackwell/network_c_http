# CHECK WE HAVE ALL THE VARIABLES WE NEED

message("test_include_dirs     ${TEST_INCLUDE_DIRS}")
message("test_link_libs        ${TEST_LINK_LIBS}")

set(TARGET test_memalloc)
add_executable(${TARGET}
	test_memalloc.c
)

target_include_directories(${TARGET} 
	PUBLIC 
		${TEST_INCLUDE_dirS})

target_link_libraries(${TARGET}
	PUBLIC ${TEST_LINK_LIBS})

add_test(${TARGET} ${TARGET})



set(TARGET2 test_memalloc_2)
add_executable(${TARGET2}
		test_memalloc_2.c
		)

target_include_directories(${TARGET2}
		PUBLIC
		${TEST_INCLUDE_dirS})

target_link_libraries(${TARGET2}
		PUBLIC ${TEST_LINK_LIBS})

add_test(${TARGET2} ${TARGET2})

