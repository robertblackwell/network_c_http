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
        ../rbl/logger.h
		../rbl/unittest.h
		socket_functions.h
		../rbl/check_tag.h
	#common
		#common/generics
		common/generics/dlist.h
		common/alloc.h
		common/url_parser.h
		common/url.h
		common/buffer_chain.h
		common/cbuffer.h
		http/header_list.h
		common/iobuffer.h
		http/kvpair.h
		common/list.h
		common/queue.h
		common/utils.h
		#http/1.1 parser
		http/message.h
		http/parser_types.h
		http/parser.h
		${EXTSRC}/llhttp/llhttp.h
	#sync/
		sync/sync.h
		sync/sync_internal.h
		sync/tags.h
		sync/sync_client.h
#		sync/sync_handler.h
#		sync/sync_handler_example.h
#		sync/worker.h
	#demo_protocol
#		demo_protocol/demo_client.h
#		demo_protocol/demo_connection.h
#		demo_protocol/demo_process_request.h
#		demo_protocol/demo_handler.h
#		demo_protocol/demo_message.h
#		demo_protocol/demo_parser.h
#		demo_protocol/demo_parser_test.h
#		demo_protocol/demo_server.h
#		demo_protocol/demo_sync_reader.h
#		demo_protocol/demo_sync_writer.h
#		demo_protocol/demo_parser_test.h
#		demo_protocol/demo_sync_reader_private.h


		#sync simple_server sync_app

	#sync verifier sync_app

	#aio runloop and aio_http/1.1 details
		runloop/runloop.h
		runloop/rl_internal.h
	#aio api
        async/async.h
		async/async_internal.h
	)

set(C_HTTP_LIBRARY_SOURCE_FILES
	# basic

	#misc
		../rbl/unittest.c
        ../rbl/logger.c
		socket_functions.c


	#http/1.1 API
		common/generics/dlist.c
		common/alloc.c
		common/buffer_chain.c
		common/cbuffer.c
		http/kvpair.c
		common/iobuffer.c
		http/header_list.c
		common/list.c
		common/url_parser.c
		common/url.c
		common/utils.c
		common/queue.c
		http/message.c
		http/parser_types.c
		http/parser.c
		${EXTSRC}/llhttp/api.c
		${EXTSRC}/llhttp/http.c
		${EXTSRC}/llhttp/llhttp.c
	#sync
		sync/sync_client.c
		sync/sync_connection.c
		sync/sync_server.c
		sync/sync_handler_example.c
		sync/worker.c

	#aio runloop and aio_http/1.1 details
		runloop/fdtable.c
		runloop/fd_map.c
		runloop/reactor.c
		runloop/w_listener.c
		runloop/w_eventfd.c
		runloop/w_timerfd.c
        runloop/w_stream.c
		runloop/w_queue.c
		runloop/interthread_queue.c
		runloop/watcher.c
		runloop/evfd_queue.c
		runloop/run_list.c
		runloop/functor.c
		runloop/functor_list.c
	#async
		async/async_server.c
        async/connection_api.c
		async/connection_cleanup.c
		async/connection_event.c
		async/connection_helpers.c
		async/connection_read.c
		async/connection_write.c
		async/async_handler.c
		async/async_socket.c
#		async/async_process_request.c
	#demo_protocol
#		demo_protocol/demo_client.c
#		demo_protocol/demo_handler.c
#		demo_protocol/demo_process_request.c
#		demo_protocol/demo_connection.c
#		demo_protocol/demo_message.c
#		demo_protocol/demo_parser.c
#		demo_protocol/demo_parser_test.c
#		demo_protocol/demo_parser_test.c
#		demo_protocol/demo_server.c
#		demo_protocol/demo_sync_reader.c
#		demo_protocol/demo_sync_writer.c
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
