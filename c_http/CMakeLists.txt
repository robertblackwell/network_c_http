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
	alloc.h
	#basic
		buffer/cbuffer.h
		buffer/buffer_chain.h
		buffer/iobuffer.h
		list.h
		kvpair.h
		queue.h
		utils.h
	#misc
		logger.h
		unittest.h
	#http/1.1 details
		datasource.h
		hdrlist.h
		ll_parser_types.h
		ll_parser.h
		parser_test.h
		rdsocket.h
		${EXTSRC}/llhttp/llhttp.h
	#http/1.1 API
		message.h
		url_parser.h
		url.h
		sync/client.h
		sync/handler.h
		sync/server.h
		sync/worker.h
		sync/writer.h

	#sync simple_server app
	sync/handler_example.h

	#sync verifier app

	#aio runloop and aio_http/1.1 details
		xr/fdtable.h
		xr/reactor.h
		xr/run_list.h
		xr/w_listener.h
		xr/w_fdevent.h
		xr/w_timer.h
		xr/w_queue.h
		xr/w_socket.h
		xr/watcher.h
		xr/evfd_queue.h
		xr/conn_list.h
	#aio api
		xr/types.h
		xr/xr_server.h
		xr/conn.h
		xr/handler.h

	)

set(C_HTTP_LIBRARY_SOURCE_FILES
	alloc.c
	# basic
		buffer/buffer_chain.c
		buffer/cbuffer.c
		buffer/iobuffer.c
		list.c
		kvpair.c
		queue.c
		utils.c

	#misc
		unittest.c
		logger.c

	#http/1.1 details
		datasource.c
		hdrlist.c
		rdsocket.c
		parser_test.c
		ll_parser_types.c
		ll_parser.c
		sync/worker.c

	#http/1.1 API
		message.c
		url_parser.c
		url.c
		sync/client.c
		sync/writer.c
		sync/ll_reader.c
		sync/server.c
		sync/handler_example.c

	#aio runloop and aio_http/1.1 details
		xr/fdtable.c
		xr/reactor.c
		xr/w_listener.c
		xr/w_fdevent.c
		xr/w_timer.c
		xr/w_socket.c
		xr/w_queue.c
		xr/watcher.c
		xr/evfd_queue.c
		xr/conn_list.c
		xr/run_list.c
		${EXTSRC}/llhttp/api.c
		${EXTSRC}/llhttp/http.c
		${EXTSRC}/llhttp/llhttp.c

	#aio_http/1.1 api
		xr/xr_server.c
		xr/conn.c
		xr/conn_read.c
		xr/conn_write.c
		xr/handler.c
		xr/handler_simple.c
		xr/handler_echo.c
		xr/handler_function.c

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
