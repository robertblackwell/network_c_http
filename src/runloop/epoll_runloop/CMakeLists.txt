set(epoll_RUNLOOP_HEADER_FILES
	#aio runloop and aio_http/1.1 details
		${CMAKE_CURRENT_SOURCE_DIR}/runloop.h
		${CMAKE_CURRENT_SOURCE_DIR}/runloop_internal.h
		${CMAKE_CURRENT_SOURCE_DIR}/rl_internal.h
		${CMAKE_CURRENT_SOURCE_DIR}/rl_events_internal.h
		${CMAKE_CURRENT_SOURCE_DIR}/event_table.h
		${CMAKE_CURRENT_SOURCE_DIR}/epoll_helper.h
	)

set(epoll_RUNLOOP_SOURCE_FILES
	#aio runloop and aio_http/1.1 details
#		 ${CMAKE_CURRENT_SOURCE_DIR}/asio_stream.c
#		${CMAKE_CURRENT_SOURCE_DIR}/asio_listener.c
#		${CMAKE_CURRENT_SOURCE_DIR}/fd_map.c
#		${CMAKE_CURRENT_SOURCE_DIR}/fdtable.c
		${CMAKE_CURRENT_SOURCE_DIR}/functor.c
		${CMAKE_CURRENT_SOURCE_DIR}/functor_list.c
		 ${CMAKE_CURRENT_SOURCE_DIR}/rl_allocator.c
        ${CMAKE_CURRENT_SOURCE_DIR}/runloop.c
		${CMAKE_CURRENT_SOURCE_DIR}/event_table.c
		${CMAKE_CURRENT_SOURCE_DIR}/epoll_helper.c

		${CMAKE_CURRENT_SOURCE_DIR}/w_watcher_base.c

		${CMAKE_CURRENT_SOURCE_DIR}/rl_user_event.c
		${CMAKE_CURRENT_SOURCE_DIR}/rl_listener.c
        ${CMAKE_CURRENT_SOURCE_DIR}/rl_queue_watcher.c
		${CMAKE_CURRENT_SOURCE_DIR}/rl_stream.c
		${CMAKE_CURRENT_SOURCE_DIR}/rl_timer.c
		${CMAKE_CURRENT_SOURCE_DIR}/rl_user_event_queue.c
		${CMAKE_CURRENT_SOURCE_DIR}/rl_interthread_queue.c

#		${CMAKE_CURRENT_SOURCE_DIR}/asio_listener.c
#		${CMAKE_CURRENT_SOURCE_DIR}/asio_stream.c
#		${CMAKE_CURRENT_SOURCE_DIR}/event_allocator.c
	)

set(RUNLOOP_SOURCE_FILES ${RUNLOOP_SOURCE_FILES} ${epoll_RUNLOOP_SOURCE_FILES} PARENT_SCOPE)
set(RUNLOOP_HEADER_FILES ${RUNLOOP_HEADER_FILES} ${epoll_RUNLOOP_HEADER_FILES} PARENT_SCOPE)
set_source_files_properties(${RUNLOOP_SOURCE_FILES} PROPERTIES LANGUAGE C)

#target_source(${C_HTTP_LIB_NAME} ${RUNLOOP_LIBRARY_SOURCE_FILES})
