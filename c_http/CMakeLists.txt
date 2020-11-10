##
## marvin/CMakeLists.txt
##
## this directory exports marvin_library
##
## Note includes 4 files from vendor/src
##
set(EXTSRC ${C_EG_VENDOR_SRCDIR})

#add_custom_command(
#	# generate mytypet list code
#	COMMAND bash ${C_EG_TOOLS_DIR}/gen_typed_list.sh generate MyType MT
#	DEPENDS list/template.h list/template.c mytype/hand_coded.h mytype/hand_coded.c
#	OUTPUT ${C_EG_SOURCE_DIR}/mytype_list.h ${C_EG_SOURCE_DIR}/mytype_list.c
#	COMMENT "Generating mytype_list.h mytype_list.c"
#	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
#)
#add_custom_command(
#	# generate kvpair list code
#	COMMAND bash ${C_EG_TOOLS_DIR}/gen_typed_list.sh generate KVPair HDR
#	DEPENDS list/template.h list/template.c headerline/hand_coded.h headerline/hand_coded.c
#	OUTPUT ${C_EG_SOURCE_DIR}/hdrlist.h ${C_EG_SOURCE_DIR}/hdrlist.c
#	COMMENT "Generating kvpair_list.h kvpair_list.c"
#	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
#)

set(C_EG_LIBRARY_HEADER_FILES
	alloc.h
	buffer/cbuffer.h
	buffer/buffer_chain.h
	buffer/iobuffer.h
	client.h
	datasource.h
	handler.h
	handler_example.h
	kvpair.h
	hdrlist.h
	list.h
	logger.h
	message.h
	# parser.h
	ll_parser_types.h
	ll_parser.h
	parser_test.h
	queue.h
	reader.h
	rdsocket.h
	server.h
#	test_helper_types.h
	unittest.h
	utils.h
	worker.h
	writer.h
	# ${EXTSRC}/http-parser/http_parser.h
	${C_EG_VENDOR_INCLUDEDIR}/llhttp.h
	)

set(C_EG_LIBRARY_SOURCE_FILES
	alloc.c
	buffer/buffer_chain.c
	buffer/cbuffer.c
	buffer/iobuffer.c
	client.c
	datasource.c
	handler_example.c
	kvpair.c
	hdrlist.c
	list.c
	logger.c
	message.c
	# parser.c
	ll_parser_types.c
	ll_parser.c
	parser_test.c
	queue.c
	reader.c
	rdsocket.c
	server.c
#	test_helper_types.c
	unittest.c
	utils.c
	worker.c
	writer.c
	# ${EXTSRC}/http-parser/http_parser.c
	${EXTSRC}/llhttp/api.c
	${EXTSRC}/llhttp/http.c
	${EXTSRC}/llhttp/llhttp.c
	)

set_source_files_properties(${C_EG_LIBRARY_SOURCE_FILES} PROPERTIES LANGUAGE C)

add_library(${C_EG_LIB_NAME} 
	STATIC 
		${C_EG_LIBRARY_SOURCE_FILES} 
		${C_EG_LIBRARY_HEADER_FILES}
)

target_include_directories(${C_EG_LIB_NAME} 
	SYSTEM PUBLIC 
	${C_EG_INCLUDE_PATHS}
)

target_link_libraries(${C_EG_LIB_NAME} ${C_EG_VENDOR_LIBDIR}/libllhttp.a)
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
