# CHECK WE HAVE ALL THE VARIABLES WE NEED


set(TARGET test_ucontext)
add_executable(${TARGET}
	test_ucontext.c
)

target_include_directories(${TARGET} 
	PUBLIC 
		${TEST_INCLUDE_dirS})

target_link_libraries(${TARGET} 
	PUBLIC ${TEST_LINK_LIBS})

add_test(${TARGET} ${TARGET})


