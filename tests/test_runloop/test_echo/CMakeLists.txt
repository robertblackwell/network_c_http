
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

add_executable(test_client client.c)
target_compile_definitions(test_client PUBLIC ASIO_LISTENER_DISABLED)
target_include_directories(test_client PUBLIC ${TEST_INCLUDE_dirS})
target_link_libraries(test_client	PUBLIC ${TEST_LINK_LIBS})
add_test(test_client test_client)

add_executable(test_server
		server.c
		listener_ctx.c
		stream_ctx.h
		stream_ctx.c
		reader.c
		writer.c
		line_parser.c
)
target_compile_definitions(test_server PUBLIC ASIO_LISTENER_DISABLED)
target_include_directories(test_server PUBLIC ${TEST_INCLUDE_dirS})
target_link_libraries(test_server	PUBLIC ${TEST_LINK_LIBS})
add_test(test_server test_server)

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
