
DEFINE_PROPERTY(DIRECTORY PROPERTY XX PROPERTY YY)
SET_PROPERTY(DIRECTORY . PROPERTY narf "thisisanarfvalue")
GET_PROPERTY(varx DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY narf)
get_directory_property(DIR_DEFS ${CMAKE_CURRENT_LIST_DIR} COMPILE_DEFINITIONS)
message("NARF  ${varx}")

set(APPLE_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(APPLE_TEST_INCLUDE_DIRS 
	${APPLE_TEST_DIR} 
	${C_HTTP_INCLUDE_PATHS} 
	SYSTEM ${VENDOR_DIR}/include)

set(APPLE_TEST_LINK_LIBS 
	${C_HTTP_LIB_NAME} 
	Threads::Threads 
	# ${CMAKE_DL_LIBS}
	# m
	# rt
	# ${CMAKE_DL_LIBS}
	# ${DL_LIB_SO}
	)
set(TEST_SERVER_BASEURL "http://whiteacorn.com/utests/echo/test.php")
add_subdirectory(test_buffers)

if (C_HTTP_Verbose)
	message("APPLE_TEST_DIR                      ${APPLE_TEST_DIR}")
	message("APPLE_TEST_LINK_LIBS                ${APPLE_TEST_LINK_LIBS}")
	message("APPLE_TEST_INLCUDE_DIRS             ${APPLE_TEST_INCLUDE_DIRS}")

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

