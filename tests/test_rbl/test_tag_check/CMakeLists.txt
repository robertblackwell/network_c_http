# CHECK WE HAVE ALL THE VARIABLES WE NEED

message("test_include_dirs     ${TEST_INCLUDE_DIRS}")
message("test_link_libs        ${TEST_LINK_LIBS}")

function(inline)
set(TARGET test_tag_check_inline)
add_executable(${TARGET}
        test_tag.c
)

target_compile_definitions(${TARGET} PUBLIC RBL_TAG_CHECK_ON RBL_CHECK_TAG_INLINE)
target_include_directories(${TARGET}
	PUBLIC 
		${TEST_INCLUDE_dirS})

target_link_libraries(${TARGET} 
	PUBLIC ${TEST_LINK_LIBS})

add_test(${TARGET} ${TARGET})
endfunction()

function(no_inline)
	set(TARGET test_tag_check_no_inline)
	add_executable(${TARGET}
			test_tag.c
	)

	target_compile_definitions(${TARGET} PUBLIC RBL_TAG_CHECK_ON)
	target_include_directories(${TARGET}
			PUBLIC
			${TEST_INCLUDE_dirS})

	target_link_libraries(${TARGET}
			PUBLIC ${TEST_LINK_LIBS})

	add_test(${TARGET} ${TARGET})
endfunction()

inline()
no_inline()