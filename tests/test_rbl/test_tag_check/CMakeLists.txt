# CHECK WE HAVE ALL THE VARIABLES WE NEED

message("test_include_dirs     ${TEST_INCLUDE_DIRS}")
message("test_link_libs        ${TEST_LINK_LIBS}")

set(TARGET test_tag_check)
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


