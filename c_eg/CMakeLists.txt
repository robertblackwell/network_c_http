##
## marvin/CMakeLists.txt
##
## this directory exports marvin_library
##
## Note includes 4 files from vendor/src
##
set(EXTSRC ${C_EG_VENDOR_SRCDIR})

set(C_EG_LIBRARY_HEADER_FILES 
	unittest.h
	buffer/contig_buffer.h
	logger.h
	server.h
	queue.h
	worker.h
	)

set(C_EG_LIBRARY_SOURCE_FILES 

	unittest.c
	# buffer/buffer_chain.cpp			
	buffer/contig_buffer.c
	logger.c		
	server.c
	queue.c
	worker.c
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
