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
	buffer/cbuffer.h
	buffer/buffer_chain.h
	buffer/iobuffer.h
	datasource.h
	kvpair.h
	hdrlist.h
	list.h
	logger.h
	message.h
	ll_parser_types.h
	ll_parser.h
	operation.h
	oprlist.h
	parser_test.h
	queue.h
	rdsocket.h
	unittest.h
	utils.h
	url_parser.h
	url.h

	sync/client.h
	sync/handler.h
	sync/handler_example.h
	sync/server.h
	sync/worker.h
	sync/writer.h

	xr/fdtable.h
	xr/reactor.h
	xr/listener.h
	xr/fdevent.h
	xr/timer_watcher.h
	xr/queue_watcher.h
	xr/socket_watcher.h
	xr/watcher.h
	xr/types.h
	xr/evfd_queue.h
	xr/xr_server.h
	xr/conn.h
	xr/conn_list.h
	xr/xr_reader.h
	xr/run_list.h
	xr/handler.h

	${EXTSRC}/llhttp/llhttp.h
	)

set(C_HTTP_LIBRARY_SOURCE_FILES
	alloc.c
	buffer/buffer_chain.c
	buffer/cbuffer.c
	buffer/iobuffer.c
	datasource.c
	kvpair.c
	hdrlist.c
	list.c
	logger.c
	message.c
	ll_parser_types.c
	ll_parser.c
	operation.c
	oprlist.c
	parser_test.c
	queue.c
	rdsocket.c
#	test_helper_types.c
	unittest.c
	utils.c
	url_parser.c
	url.c

	sync/client.c
	sync/writer.c
	sync/ll_reader.c
	sync/server.c
	sync/worker.c
	sync/handler_example.c

	xr/xr_server.c
	xr/fdtable.c
	xr/reactor.c
	xr/listener.c
	xr/fdevent.c
	xr/timer_watcher.c
	xr/socket_watcher.c
	xr/queue_watcher.c
	xr/watcher.c
	xr/evfd_queue.c
	xr/conn.c
	xr/conn_read.c
	xr/conn_write.c
	xr/conn_list.c
	xr/run_list.c
	xr/xr_reader.c
	xr/handler.c
	xr/handler_simple.c
	xr/handler_echo.c
	xr/handler_function.c

	${EXTSRC}/llhttp/api.c
	${EXTSRC}/llhttp/http.c
	${EXTSRC}/llhttp/llhttp.c
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
