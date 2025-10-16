set(HEADER_FILES
#        ${CMAKE_CURRENT_SOURCE_DIR}/generics/dlist.h
        ${CMAKE_CURRENT_SOURCE_DIR}/alloc.h
        ${CMAKE_CURRENT_SOURCE_DIR}/url_parser.h
        ${CMAKE_CURRENT_SOURCE_DIR}/url.h
        ${CMAKE_CURRENT_SOURCE_DIR}/buffer_chain.h
        ${CMAKE_CURRENT_SOURCE_DIR}/cbuffer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/iobuffer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/list.h
        ${CMAKE_CURRENT_SOURCE_DIR}/queue.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils.h
        ${CMAKE_CURRENT_SOURCE_DIR}/make_uuid.h
        ../../apps/client/verify_statistics.h
)
set(SOURCE_FILES
#        ${CMAKE_CURRENT_SOURCE_DIR}/generics/dlist.c
        ${CMAKE_CURRENT_SOURCE_DIR}/alloc.c
        ${CMAKE_CURRENT_SOURCE_DIR}/buffer_chain.c
        ${CMAKE_CURRENT_SOURCE_DIR}/cbuffer.c
        ${CMAKE_CURRENT_SOURCE_DIR}/iobuffer.c
        ${CMAKE_CURRENT_SOURCE_DIR}/list.c
        ${CMAKE_CURRENT_SOURCE_DIR}/url_parser.c
        ${CMAKE_CURRENT_SOURCE_DIR}/url.c
        ${CMAKE_CURRENT_SOURCE_DIR}/utils.c
        ${CMAKE_CURRENT_SOURCE_DIR}/queue.c
        ${CMAKE_CURRENT_SOURCE_DIR}/make_uuid.c
        ../../apps/client/verify_statistics.c

)

set_source_files_properties(${SOURCE_FILES} PROPERTIES LANGUAGE C)
set(C_HTTP_LIBRARY_SOURCE_FILES ${C_HTTP_LIBRARY_SOURCE_FILES} ${SOURCE_FILES} PARENT_SCOPE)
set(C_HTTP_LIBRARY_HEADER_FILES ${C_HTTP_LIBRARY_HEADER_FILES} ${HEADER_FILES} PARENT_SCOPE)

#target_source(${C_HTTP_LIB_NAME} ${RUNLOOP_LIBRARY_SOURCE_FILES})
