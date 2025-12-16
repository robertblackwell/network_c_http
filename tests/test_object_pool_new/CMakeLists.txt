# CHECK WE HAVE ALL THE VARIABLES WE NEED

message("test_include_dirs     ${TEST_INCLUDE_DIRS}")
message("test_link_libs        ${TEST_LINK_LIBS}")

if(0)
set(TARGET test_object_pool_new)
add_executable(${TARGET}
	test_object_pool.c
	${C_HTTP_SOURCE_DIR}/allocators/object_pool/alloc_object_pool.h
	${C_HTTP_SOURCE_DIR}/allocators/object_pool/alloc_object_pool_internal.h
	${C_HTTP_SOURCE_DIR}/allocators/object_pool/object_pool.c
	${C_HTTP_SOURCE_DIR}/allocators/object_pool/intrusive_list.c
	${C_HTTP_SOURCE_DIR}/allocators/object_pool/memory_block.c
	${C_HTTP_SOURCE_DIR}/allocators/object_pool/alloc_object_pool.h
)
target_include_directories(${TARGET} PUBLIC ${TEST_INCLUDE_dirS})
target_link_libraries(${TARGET} PUBLIC ${TEST_LINK_LIBS})
add_test(${TARGET} ${TARGET})
endif()

function(build  target_name compile_option)
	set(TARGET test_object_pool_${target})
	add_executable(${target_name}
			test_object_pool.c
			${C_HTTP_SOURCE_DIR}/allocators/object_pool/alloc_object_pool.h
			${C_HTTP_SOURCE_DIR}/allocators/object_pool/alloc_object_pool_internal.h
			${C_HTTP_SOURCE_DIR}/allocators/object_pool/object_pool.c
			${C_HTTP_SOURCE_DIR}/allocators/object_pool/intrusive_list.c
			${C_HTTP_SOURCE_DIR}/allocators/object_pool/memory_block.c
			${C_HTTP_SOURCE_DIR}/allocators/object_pool/alloc_object_pool.h
	)
	target_include_directories(${target_name} PUBLIC ${TEST_INCLUDE_DIRS})
	target_link_libraries(${target_name} PUBLIC ${TEST_LINK_LIBS})
	target_compile_definitions(${target_name} PRIVATE ${compile_option})
	message(" ")
	message("BBBBBBBBBBbuilding ${target_name} compile_definitions ${compile_option}")
	message(" ")
	add_test(${target_name} ${target_name})
endfunction()
build(test_object_pool_debug "OBJECT_POOL_DEBUG")
build(test_object_pool_nodebug "OBJECT_POOL_NODEBUG")
