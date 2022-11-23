##
## marvin/CMakeLists.txt
##
## this directory exports marvin_library
##
## Note includes 4 files from vendor/src
##
set(EXTSRC ${C_HTTP_VENDOR_SRCDIR})

#add_custom_command(
#	# generate mytypet list code
#	COMMAND bash ${C_HTTP_TOOLS_DIR}/gen_typed_list.sh generate MyType MT
#	DEPENDS list/template.h list/template.c mytype/hand_coded.h mytype/hand_coded.c
#	OUTPUT ${C_HTTP_SOURCE_DIR}/mytype_list.h ${C_HTTP_SOURCE_DIR}/mytype_list.c
#	COMMENT "Generating mytype_list.h mytype_list.c"
#	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
#)
#add_custom_command(
#	# generate kvpair list code
#	COMMAND bash ${C_HTTP_TOOLS_DIR}/gen_typed_list.sh generate KVPair HDR
#	DEPENDS list/template.h list/template.c headerline/hand_coded.h headerline/hand_coded.c
#	OUTPUT ${C_HTTP_SOURCE_DIR}/hdrlist.h ${C_HTTP_SOURCE_DIR}/hdrlist.c
#	COMMENT "Generating kvpair_list.h kvpair_list.c"
#	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
#)

set(C_HTTP_LIBRARY_HEADER_FILES
	#basic data structures
	#misc
		logger.h
		unittest.h
	#common
		common/alloc.h
		common/message.h
		common/url_parser.h
		common/url.h
		common/buffer_chain.h
		common/cbuffer.h
		common/hdrlist.h
		common/iobuffer.h
		common/kvpair.h
		common/list.h
		common/queue.h
		common/utils.h
		#http/1.1 parser
		common/http_parser/ll_parser_types.h
		common/http_parser/ll_parser.h
		common/http_parser/parser_test.h
		common/http_parser/rdsocket.h
		common/http_parser/datasource.h
		${EXTSRC}/llhttp/llhttp.h
	#sync/
		sync/sync_client.h
		sync/sync_server.h
		sync/sync_writer.h
		sync/sync_reader.h
		sync/sync_handler.h
		sync/sync_handler_example.h
		sync/worker.h
	#demo_protocol
		demo_protocol/demo_client.h
		demo_protocol/demo_handler.h
		demo_protocol/demo_message.h
		demo_protocol/demo_parser.h
		demo_protocol/demo_parser_test.h
		demo_protocol/demo_server.h
		demo_protocol/demo_sync_reader.h
		demo_protocol/demo_sync_writer.h
		demo_protocol/demo_parser_test.h
		demo_protocol/demo_sync_reader_private.h


		#sync simple_server app

	#sync verifier app

	#aio simple_runloop and aio_http/1.1 details
		simple_runloop/runloop.h
		simple_runloop/rl_internal.h
	#aio api
        async/tcp_conn_list.h
		async/types.h
		async/async_server.h
        async/tcp_conn.h
		async/handler.h

	)

set(C_HTTP_LIBRARY_SOURCE_FILES
	# basic

	#misc
		unittest.c
		logger.c


	#http/1.1 API
		common/alloc.c
		common/buffer_chain.c
		common/cbuffer.c
		common/kvpair.c
		common/iobuffer.c
		common/hdrlist.c
		common/list.c
		common/message.c
		common/url_parser.c
		common/url.c
		common/utils.c
		common/queue.c
		#http/1.1 details
		common/http_parser/datasource.c
		common/http_parser/rdsocket.c
		common/http_parser/parser_test.c
		common/http_parser/ll_parser_types.c
		common/http_parser/ll_parser.c
		${EXTSRC}/llhttp/api.c
		${EXTSRC}/llhttp/http.c
		${EXTSRC}/llhttp/llhttp.c
	#sync
		sync/sync_client.c
		sync/sync_writer.c
		sync/sync_reader.c
		sync/sync_server.c
		sync/sync_handler_example.c
		sync/worker.c

	#aio simple_runloop and aio_http/1.1 details
		simple_runloop/fdtable.c
		simple_runloop/fd_map.c
		simple_runloop/reactor.c
		simple_runloop/w_listener.c
		simple_runloop/w_eventfd.c
		simple_runloop/w_timerfd.c
        simple_runloop/w_stream.c
		simple_runloop/w_queue.c
#		simple_runloop/interthread_queue.c
		simple_runloop/watcher.c
		simple_runloop/evfd_queue.c
		simple_runloop/run_list.c
		simple_runloop/functor.c
		simple_runloop/functor_list.c
	#async
		async/async_server.c
        async/tcp_conn.c
		async/tcp_conn_read.c
		async/tcp_conn_read_msg.c
		async/tcp_conn_write.c
        async/tcp_conn_list.c
		async/handler.c
		async/handler_simple.c
		async/handler_echo.c
		async/handler_function.c
	#demo_protocol
		demo_protocol/demo_client.c
		demo_protocol/demo_handler.c
		demo_protocol/demo_message.c
		demo_protocol/demo_parser.c
		demo_protocol/demo_parser_test.c
		demo_protocol/demo_server.c
		demo_protocol/demo_sync_reader.c
		demo_protocol/demo_sync_writer.c
	#async_demo
#		async_demo/demo_server.c
#		async_demo/demo_handler.c
	)

set_source_files_properties(${C_HTTP_LIBRARY_SOURCE_FILES} PROPERTIES LANGUAGE C)

add_library(${C_HTTP_LIB_NAME} 
	STATIC 
		${C_HTTP_LIBRARY_SOURCE_FILES} 
		${C_HTTP_LIBRARY_HEADER_FILES}
)

target_include_directories(${C_HTTP_LIB_NAME} 
	SYSTEM PUBLIC 
	${C_HTTP_INCLUDE_PATHS}
)

# target_precompile_headers(marvin_library PUBLIC
#   	<marvin/boost_stuff.hpp>
# 	<memory>
# 	<stddef.h>
# 	<vector>
# 	<string>
# 	<iostream>
# 	<sstream>
# 	<functional>
# 	<iterator>
# 	<thread>
# 	<pthread.h>
# 	<regex>
# 	<map>
# 	<boost/asio.hpp>
# 	<boost/asio/ssl.hpp>
# 	<boost/system/error_code.hpp>
# 	<boost/asio/error.hpp>
# 	<boost/bind.hpp>
# 	<boost/function.hpp>
# 	<boost/date_time/posix_time/posix_time.hpp>
# 	<boost/algorithm/string.hpp>
# 	<boost/uuid/uuid.hpp>
# 	<boost/uuid/uuid_generators.hpp>
# 	<boost/uuid/uuid_io.hpp>
# 	<boost/filesystem.hpp>
# 	<boost/unordered_set.hpp>	
# 	<marvin/error/marvin_error.hpp>
# 	<boost/process.hpp>
# 	)
