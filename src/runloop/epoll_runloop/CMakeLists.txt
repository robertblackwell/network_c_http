set(epoll_RUNLOOP_HEADER_FILES
	#aio runloop and aio_http/1.1 details
		${CMAKE_CURRENT_SOURCE_DIR}/runloop.h
			${CMAKE_CURRENT_SOURCE_DIR}/w_eventfd.h
			${CMAKE_CURRENT_SOURCE_DIR}/w_eventfd_queue.h
			${CMAKE_CURRENT_SOURCE_DIR}/w_listener.h
			${CMAKE_CURRENT_SOURCE_DIR}/w_stream.h
			${CMAKE_CURRENT_SOURCE_DIR}/w_timer.h
#			${CMAKE_CURRENT_SOURCE_DIR}/event_allocator.h
		${CMAKE_CURRENT_SOURCE_DIR}/rl_internal.h
	)

set(epoll_RUNLOOP_SOURCE_FILES
	#aio runloop and aio_http/1.1 details
		# ${CMAKE_CURRENT_SOURCE_DIR}/asio_stream.c
		${CMAKE_CURRENT_SOURCE_DIR}/w_eventfd_queue.c
		${CMAKE_CURRENT_SOURCE_DIR}/fd_map.c
		${CMAKE_CURRENT_SOURCE_DIR}/fdtable.c
		${CMAKE_CURRENT_SOURCE_DIR}/functor.c
		${CMAKE_CURRENT_SOURCE_DIR}/functor_list.c
		${CMAKE_CURRENT_SOURCE_DIR}/w_interthread_queue.c
		# ${CMAKE_CURRENT_SOURCE_DIR}/run_list.c
        ${CMAKE_CURRENT_SOURCE_DIR}/runloop.c
		${CMAKE_CURRENT_SOURCE_DIR}/w_eventfd.c
		${CMAKE_CURRENT_SOURCE_DIR}/w_listener.c
        ${CMAKE_CURRENT_SOURCE_DIR}/w_queue_watcher.c
		${CMAKE_CURRENT_SOURCE_DIR}/w_stream.c
		${CMAKE_CURRENT_SOURCE_DIR}/w_timerfd.c
		${CMAKE_CURRENT_SOURCE_DIR}/w_watcher_base.c
#		${CMAKE_CURRENT_SOURCE_DIR}/event_allocator.c
	)

set(RUNLOOP_SOURCE_FILES ${RUNLOOP_SOURCE_FILES} ${epoll_RUNLOOP_SOURCE_FILES} PARENT_SCOPE)
set(RUNLOOP_HEADER_FILES ${RUNLOOP_HEADER_FILES} ${epoll_RUNLOOP_HEADER_FILES} PARENT_SCOPE)
set_source_files_properties(${RUNLOOP_SOURCE_FILES} PROPERTIES LANGUAGE C)

#target_source(${C_HTTP_LIB_NAME} ${RUNLOOP_LIBRARY_SOURCE_FILES})
