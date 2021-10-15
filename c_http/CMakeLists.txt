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
		dsl/alloc.h
		dsl/list.h
		dsl/kvpair.h
		dsl/queue.h
		dsl/utils.h
	#misc
		logger.h
		unittest.h
	#http/1.1 details
		details/hdrlist.h
		details/ll_parser_types.h
		details/ll_parser.h
		details/parser_test.h
		details/rdsocket.h
		details/datasource.h
		details/worker.h
		${EXTSRC}/llhttp/llhttp.h
	#http/1.1 API
		api/message.h
		api/url_parser.h
		api/url.h
		api/client.h
		api/handler.h
		api/server.h
		api/writer.h
		api/handler_example.h
		api/cbuffer.h
		api/buffer_chain.h
		api/iobuffer.h

	#sync simple_server app

	#sync verifier app

	#aio runloop and aio_http/1.1 details
		runloop/fdtable.h
		runloop/reactor.h
		runloop/run_list.h
		runloop/w_listener.h
		runloop/w_fdevent.h
		runloop/w_timer.h
		runloop/w_queue.h
		runloop/w_socket.h
		runloop/watcher.h
		runloop/evfd_queue.h
		runloop/conn_list.h
	#aio api
		aio_api/types.h
		aio_api/xr_server.h
		aio_api/conn.h
		aio_api/handler.h

	)

set(C_HTTP_LIBRARY_SOURCE_FILES
	# basic
		dsl/alloc.c
		dsl/list.c
		dsl/kvpair.c
		dsl/queue.c
		dsl/utils.c

	#misc
		unittest.c
		logger.c

	#http/1.1 details
		details/datasource.c
		details/hdrlist.c
		details/rdsocket.c
		details/parser_test.c
		details/ll_parser_types.c
		details/ll_parser.c
		details/worker.c
		${EXTSRC}/llhttp/api.c
		${EXTSRC}/llhttp/http.c
		${EXTSRC}/llhttp/llhttp.c

	#http/1.1 API
		api/message.c
		api/url_parser.c
		api/url.c
		api/client.c
		api/writer.c
		api/reader.c
		api/server.c
		api/handler_example.c
		api/buffer_chain.c
		api/cbuffer.c
		api/iobuffer.c

	#aio runloop and aio_http/1.1 details
		runloop/fdtable.c
		runloop/reactor.c
		runloop/w_listener.c
		runloop/w_fdevent.c
		runloop/w_timer.c
		runloop/w_socket.c
		runloop/w_queue.c
		runloop/watcher.c
		runloop/evfd_queue.c
		runloop/conn_list.c
		runloop/run_list.c

	#aio_http/1.1 api
		aio_api/xr_server.c
		aio_api/conn.c
		aio_api/conn_read.c
		aio_api/conn_read_msg.c
		aio_api/conn_write.c
		aio_api/handler.c
		aio_api/handler_simple.c
		aio_api/handler_echo.c
		aio_api/handler_function.c

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
