# CHECK WE HAVE ALL THE VARIABLES WE NEED

message("WWWWWtest_include_dirs     ${TEST_INCLUDE_DIRS}")
message("WWWWWtest_link_libs        ${TEST_LINK_LIBS}")

function(build  target_name compile_option)
set(TARGET test_arena_${target})
add_executable(${target_name}
        test_arena.c
        ${C_HTTP_SOURCE_DIR}/allocators/arena/arena.h
        ${C_HTTP_SOURCE_DIR}/allocators/arena/arena.c
        ${C_HTTP_SOURCE_DIR}/allocators/arena/allocated_memory.c
        ${C_HTTP_SOURCE_DIR}/allocators/arena/arena_api.c
        ${C_HTTP_SOURCE_DIR}/allocators/arena/arena_internal.h
        ${C_HTTP_SOURCE_DIR}/allocators/arena/block.c
)
target_include_directories(${target_name} PUBLIC ${TEST_INCLUDE_DIRS})
target_link_libraries(${target_name} PUBLIC ${TEST_LINK_LIBS})
target_compile_definitions(${target_name} PRIVATE ${compile_option})
message(" ")
message("BBBBBBBBBBbuilding ${target_name} compile_definitions ${compile_option}")
message(" ")
add_test(${target_name} ${target_name})
endfunction()

build(test_arena_debug "ARENA_DEBUG")
build(test_arena_nodebug ARENA_NODEBUG)



