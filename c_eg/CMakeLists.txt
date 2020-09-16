##
## marvin/CMakeLists.txt
##
## this directory exports marvin_library
##
## Note includes 4 files from vendor/src
##
set(EXTSRC ${C_EG_VENDOR_SRCDIR})

add_custom_command(
	# generate socket list code
	COMMAND echo "Makeing socket_list  class"
	DEPENDS __list_template.h list_template.c sock_list_in.h sock_list_in.c
	OUTPUT sock_list.h sock_list.c
	COMMENT "Generatiing socket_list.h socket_list.c "
)
#add_custom_target(libfoo_untar DEPENDS ${LIBFOO_TAR_HEADERS})

set(C_EG_LIBRARY_HEADER_FILES
	sock_list.h
	unittest.h
	buffer/contig_buffer.h
	logger.h
	server.h
	queue.h
	worker.h
	list.h
	)

set(C_EG_LIBRARY_SOURCE_FILES 
	sock_list.c
	unittest.c
	# buffer/buffer_chain.cpp			
	buffer/contig_buffer.c
	logger.c		
	server.c
	queue.c
	worker.c
	list.c
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
