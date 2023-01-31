# CHECK WE HAVE ALL THE VARIABLES WE NEED

message("test_include_dirs     ${TEST_INCLUDE_DIRS}")
message("test_link_libs        ${TEST_LINK_LIBS}")

set(TARGET test_generic_list)
add_executable(${TARGET}
	mytype_A.c
	mytype_B.c
	mytype_A_dlist.c
	mytype_B_dlist.c
	mytype_wrapped_dlist.c
	mytype_circular_list.c
	test_generic_list.c
)

target_include_directories(${TARGET} 
	PUBLIC 
		${TEST_INCLUDE_dirS})

target_link_libraries(${TARGET} 
	PUBLIC ${TEST_LINK_LIBS})

add_test(${TARGET} ${TARGET})


