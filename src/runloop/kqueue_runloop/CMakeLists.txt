set(kq_RUNLOOP_HEADER_FILES
	#aio runloop and aio_http/1.1 details
		${CMAKE_CURRENT_SOURCE_DIR}/runloop.h
			# ${CMAKE_CURRENT_SOURCE_DIR}/user_event.h
			# ${CMAKE_CURRENT_SOURCE_DIR}/user_event_queue.h
			# ${CMAKE_CURRENT_SOURCE_DIR}/listener.h
			# ${CMAKE_CURRENT_SOURCE_DIR}/stream.h
			${CMAKE_CURRENT_SOURCE_DIR}/rl_checktag.h
		${CMAKE_CURRENT_SOURCE_DIR}/rl_internal.h
		${CMAKE_CURRENT_SOURCE_DIR}/event_allocator.h
	)

set(kq_RUNLOOP_SOURCE_FILES
	#aio runloop and aio_http/1.1 details
		${CMAKE_CURRENT_SOURCE_DIR}/asio_stream.c
		# ${CMAKE_CURRENT_SOURCE_DIR}/user_event_queue.c
		${CMAKE_CURRENT_SOURCE_DIR}/fd_map.c
		# ${CMAKE_CURRENT_SOURCE_DIR}/fdtable.c
		${CMAKE_CURRENT_SOURCE_DIR}/functor.c
		${CMAKE_CURRENT_SOURCE_DIR}/functor_list.c
		# ${CMAKE_CURRENT_SOURCE_DIR}/interthread_queue.c
#		${CMAKE_CURRENT_SOURCE_DIR}/run_list.c
        ${CMAKE_CURRENT_SOURCE_DIR}/runloop.c
		# ${CMAKE_CURRENT_SOURCE_DIR}/user_event.c
		${CMAKE_CURRENT_SOURCE_DIR}/listener.c
        # ${CMAKE_CURRENT_SOURCE_DIR}/queue_watcher.c
		${CMAKE_CURRENT_SOURCE_DIR}/stream.c
		${CMAKE_CURRENT_SOURCE_DIR}/timer.c
		${CMAKE_CURRENT_SOURCE_DIR}/watcher_base.c
		${CMAKE_CURRENT_SOURCE_DIR}/event_allocator.c
	)

set_source_files_properties(${RUNLOOP_SOURCE_FILES} PROPERTIES LANGUAGE C)
set(RUNLOOP_SOURCE_FILES ${RUNLOOP_SOURCE_FILES} ${kq_RUNLOOP_SOURCE_FILES} PARENT_SCOPE)
set(RUNLOOP_HEADER_FILES ${RUNLOOP_HEADER_FILES} ${kq_RUNLOOP_HEADER_FILES} PARENT_SCOPE)

#target_source(${C_HTTP_LIB_NAME} ${RUNLOOP_LIBRARY_SOURCE_FILES})
