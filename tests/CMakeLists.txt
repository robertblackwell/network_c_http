


set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(TEST_INCLUDE_DIRS 
	${TEST_DIR} 
	${C_EG_INCLUDE_PATHS} 
	SYSTEM ${VENDOR_DIR}/include)

set(TEST_LINK_LIBS 
	${C_EG_LIB_NAME} 
	Threads::Threads 
	${CMAKE_DL_LIBS}
	m
	rt
	${CMAKE_DL_LIBS}
	${DL_LIB_SO}
	)
set(TEST_SERVER_BASEURL "http://whiteacorn.com/utests/echo/test.php")
add_subdirectory(test_buffers)
add_subdirectory(test_check_macro)
add_subdirectory(test_server)
add_subdirectory(test_url)
add_subdirectory(test_list)
add_subdirectory(test_header_line)
add_subdirectory(test_hdrlist)
add_subdirectory(test_parser)

if (OFF)
endif()

if (C_EG_Verbose)
	message("TEST_DIR                      ${TEST_DIR}")
	message("HEADER_DIRS                   ${HEADER_DIRS}")
	message("TEST_INLCUDE_DIRS             ${TEST_INCLUDE_DIRS}")

endif()
# these are not really 'test' in most cases they are experiments to try out an idea 
# or practice using a facility - such as json or strand
# x_test_request 		# same comment as test_x_client_request - not sure which one will survive
# x_test_ssl_server 	# boost server example implemented
# x_test_strand 		# experiment with strand
# x_test_tcp			# possibly will be devloped into a real test not sure its needed
# x_test_client_request # decome a real tests as more marvin/client/request* progresses 
# x_test_connect        # probably deprecated
# x_test_helpers		# need to investigate whther the helpers any required anymore
# x_test_json			# expploring nlohmann_json
# x_test_proxy			# probably needs to be resurrected
# x_test_timer			# experiment with timers and weak references

