
#set(test_listener_src
#		test_listener.c
#		listener_ctx.h
#		listener_ctx.c
#		connector_ctx.h
#		connector_ctx.c
#)
#set(test_listener_asio_src
#		test_listener.c
#		listener_ctx.h
#		asio_listener_ctx.c
#		connector_ctx.h
#		connector_ctx.c
#)


add_executable(test_listener
		test_listener.c
		listener_ctx.h
		listener_ctx.c
		connector_ctx.h
		connector_ctx.c
)
target_compile_definitions(test_listener PUBLIC ASIO_LISTENER_DISABLED)
target_include_directories(test_listener PUBLIC ${TEST_INCLUDE_dirS})
target_link_libraries(test_listener	PUBLIC ${TEST_LINK_LIBS})
add_test(test_listener test_listener)

# add_executable(test_listener_asio
# 		test_listener_asio.c
# 		listener_ctx.h
# 		asio_listener_ctx.c
# 		connector_ctx.h
# 		connector_ctx.c
# )
# target_compile_definitions(test_listener_asio PUBLIC ASIO_LISTENER_ENABLED)
# target_include_directories(test_listener_asio PUBLIC ${TEST_INCLUDE_dirS})
# target_link_libraries(test_listener_asio	PUBLIC ${TEST_LINK_LIBS})
# add_test(test_listener_asio test_listener_asio)
