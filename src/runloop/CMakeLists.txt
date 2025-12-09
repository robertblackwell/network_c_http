set(RUNLOOP_HEADER_FILES "")
set(RUNLOOP_SOURCE_FILES "")
set(RUNLOOP_HELPER_HEADER_FILES "")
set(RUNLOOP_HELPER_SOURCE_FILES "")

if(LINUX_FLAG)
	message("Include epoll_runloop")
	add_subdirectory(epoll_runloop)
elseif(APPLE_FLAG)
	message("Include kqueue_runloop")
	add_subdirectory(kqueue_runloop)
endif()
#set(RUNLOOP_HELPER_SOURCE_FILES epoll_runloop/event_table.c)
#set(RUNLOOP_HELPER_HEADER_FILES epoll_runloop/event_table.h)

set(C_HTTP_LIBRARY_SOURCE_FILES ${C_HTTP_LIBRARY_SOURCE_FILES} 
	${CMAKE_CURRENT_SOURCE_DIR}/functor.c
	${CMAKE_CURRENT_SOURCE_DIR}/functor_list.c
	# ${CMAKE_CURRENT_SOURCE_DIR}/event_table.c
	# ${CMAKE_CURRENT_SOURCE_DIR}/rl_allocator.c
	${RUNLOOP_HELPER_SOURCE_FILES}
	${RUNLOOP_SOURCE_FILES} 
	PARENT_SCOPE)
set(C_HTTP_LIBRARY_HEADER_FILES ${C_HTTP_LIBRARY_HEADER_FILES}
	${CMAKE_CURRENT_SOURCE_DIR}/runloop.h
	${CMAKE_CURRENT_SOURCE_DIR}/runloop_defines.h
	${CMAKE_CURRENT_SOURCE_DIR}/runloop_api.h
	${CMAKE_CURRENT_SOURCE_DIR}/functor.h
	${CMAKE_CURRENT_SOURCE_DIR}/rl_internal.h
	${CMAKE_CURRENT_SOURCE_DIR}/rl_checktag.h
	# ${CMAKE_CURRENT_SOURCE_DIR}/event_table.h
	# ${CMAKE_CURRENT_SOURCE_DIR}/rl_allocator.h
	${RUNLOOP_HELPER_HEADER_FILES}
	${RUNLOOP_HEADER_FILES} 
	PARENT_SCOPE)

message("RUNLOOP_SOURCE_FILES")
foreach(F in ${RUNLOOP_SOURCE_FILES})
	message("SOURCE FILE: ${F}")
endforeach()
