# CHECK WE HAVE ALL THE VARIABLES WE NEED

message("test_include_dirs     ${TEST_INCLUDE_DIRS}")
message("test_link_libs        ${TEST_LINK_LIBS}")

set(TARGET test_generic)
add_executable(${TARGET}
		array.c
		array.h
	mytype.c
		mytype.h
	mytype_table.c
		mytype_table.h
	test_list.c
)

target_include_directories(${TARGET} 
	PUBLIC 
		${TEST_INCLUDE_dirS})

target_link_libraries(${TARGET} 
	PUBLIC ${TEST_LINK_LIBS})

add_test(${TARGET} ${TARGET})


