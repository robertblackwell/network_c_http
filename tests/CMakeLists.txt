
DEFINE_PROPERTY(DIRECTORY PROPERTY XX PROPERTY YY)
SET_PROPERTY(DIRECTORY . PROPERTY narf "thisisanarfvalue")
GET_PROPERTY(varx DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY narf)
get_directory_property(DIR_DEFS ${CMAKE_CURRENT_LIST_DIR} COMPILE_DEFINITIONS)
message("NARF  ${varx}")

set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(TEST_INCLUDE_DIRS 
	${TEST_DIR} 
	${C_HTTP_INCLUDE_PATHS} 
	SYSTEM ${VENDOR_DIR}/include)

set(TEST_LINK_LIBS 
	${C_HTTP_LIB_NAME} 
	Threads::Threads 
	${CMAKE_DL_LIBS}
	m
	rt
	${CMAKE_DL_LIBS}
	${DL_LIB_SO}
	)
set(TEST_SERVER_BASEURL "http://whiteacorn.com/utests/echo/test.php")
add_subdirectory(test_buffers)
#add_subdirectory(test_generic) #does not build
#add_subdirectory(test_generic_list)
#add_subdirectory(test_generic_stack)
add_subdirectory(test_hdrlist)
#add_subdirectory(test_istype)
add_subdirectory(test_kvpair)
add_subdirectory(test_list)
add_subdirectory(test_list_macro)
#add_subdirectory(test_memalloc) #does not build
add_subdirectory(test_http_message)
#add_subdirectory(test_own)
add_subdirectory(test_http_parser)
add_subdirectory(test_query)
add_subdirectory(test_queue)

add_subdirectory(test_rbl)
add_subdirectory(test_runloop)

#add_subdirectory(test_rw)
# add_subdirectory(test_static_list)
#add_subdirectory(test_url) #this is superceded by test_url_parser
add_subdirectory(test_url_parser)
add_subdirectory(test_stx_msg_parser) #does not build
#add_subdirectory(test_demo/test_demo_connection)

if (OFF)
endif()

if (C_HTTP_Verbose)
	message("TEST_DIR                      ${TEST_DIR}")
	message("HEADER_DIRS                   ${HEADER_DIRS}")
	message("TEST_INLCUDE_DIRS             ${TEST_INCLUDE_DIRS}")

endif()

# these are not really 'test' in most cases they are experiments to try out an idea 
# or practice using a facility - such as json or strand
# x_test_request 		# same comment as test_x_client_request - not sure which one will survive
# x_test_ssl_server 	# boost server example implemented
# x_test_strand 		# experiment with strand
# x_test_tcp			# possibly will be developed into a real test not sure its needed
# x_test_client_request # decome a real tests as more marvin/client/request* progresses 
# x_test_connect        # probably deprecated
# x_test_helpers		# need to investigate whther the helpers any required anymore
# x_test_json			# expploring nlohmann_json
# x_test_proxy			# probably needs to be resurrected
# x_test_timer			# experiment with timers and weak references

