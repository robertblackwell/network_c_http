#ifndef c_c_http_buffer_iobuffer_h
#define c_c_http_buffer_iobuffer_h



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <common/cbuffer.h>
#define IOBUFFER_DEFAULT_CAPACITY 4*1024


/**
 * @addtogroup group_iobuffer
 * IOBuffer is intended for:
 * -   reading/writing data from/to say a socket and at the
 *      same time consuming some or all of the read data or producing more write data.
 *      The process would be something like:
 *
 *```c
 *      bytes_read = read(fd, IOBuffer_space(iob), IOBUffer_space_len(iob);
 *      IOBuffer_commit(iob, bytes_read)
 *      bytes_processed = process_bytes(..... IOBuffer_data(iob). IOBuffer_data_len(iob))
 *      IOBuffer_consume(iob, bytes_processed)
 *```
 *
 * ```c
 *      bytes_generated = output_generator( .... IOBuffer_space(iob), IOBUffer_spacelen(iob))
 *      IOBUffer_commit(iob, bytes_generated)
 *      bytes_written = write(fd, IOBuffer_data(iob), IOBuffer_datalen(iob))
 *      IOBuffer_consume(iob, bytes_written)
 *```
 *
 * NOTE: IOBuffers never (really ?) expand - they can be made to have any capacity needed at creation time, there after
 * they cannot expand. A consequence of iob is that there re no "append" style methods.
 *
 * It would be dangerous to allow a buffer to expand (and the address of the underlying memory possibly change)
 * while the same buffer was being used for IO
 *
 * NOTE: iobuffers are NOT thread safe and should not shared between threads.
 *
 * @{*/

/**
 * @brief IOBuffer as an opaque object.
 */
typedef struct IOBuffer_s IOBuffer, *IOBufferRef;

IOBufferRef IOBuffer_init(IOBufferRef iob, size_t capacity);
/**
 * @brief Create a new IOBuffer with at least the requested capacity in bytes.
 *
 * @param capacity size_t The capacity of the new IOBuffer in bytes.
 * @return IOBufferRef
 */
IOBufferRef IOBuffer_new_with_capacity(size_t capacity);
void IOBuffer_expand_and_reset(IOBufferRef iob, size_t new_capacity);
/**
 * @brief Create a new IOBuffer with a a default capacity.
 *
 * @return IOBufferRef
 */
IOBufferRef IOBuffer_new();

/**
 * @brief Makes an IOBuffer from the content of a Cbuffer by COPY
 * @param cbuf  CbufferRef
 * @return IOBufferRef
 *
 * WARNING - AT SOME POINT THIS FUNCTION WILL ACQUIRE MOVE SEMANTICS
 * AND THE SOURCE Cbuffer will be left
 *  either
 *      A) consistent but EMPTY
 *      B) undefined - to do iob we will need a new Cbuffer method called Cbuffer_steal_content
 *          needs more thinking about
 *
 */
IOBufferRef IOBuffer_from_cbuffer(CbufferRef cbuf);

/**
 * @brief Makes an IOBUffer from the a pointer and length by COPY
 * @param buf char* pointing to start of data to put in IOBUffer
 * @param len size_t   length of data
 * @return IOBufferRef
 */
IOBufferRef IOBuffer_from_buf(char* buf, size_t len);

/**
 * @brief Makes an IOBuffer from a c-string by COPY
 * @param cbuf  CbufferRef
 * @return IOBufferRef
 */
IOBufferRef IOBuffer_from_cstring(char* cstr);
/**
 * @brief Returns a c string ref to internal data
 * @param iob IOBuffer
 * @return c string Weak reference do not free
 */
const char* IOBuffer_cstr(IOBufferRef iob);
/**
 * @brief Duplicate an IOBuffer including copying the content to a new memory allocation.
 * @param iob IOBUfferRef
 * @return IOBufferRef
 */
IOBufferRef IOBuffer_dup(IOBufferRef iob);
void IOBuffer_set_used(IOBufferRef iob, size_t bytes_used);
/**
 * @brief Returns a reference pointer to the start of active data in the buffer.
 * The memory pointed into is owned by the IoBuffer. Do not free
 * @param iob
 * @return void*
 */
void* IOBuffer_data(const IOBufferRef iob);
/**
 * @brief Returns a the length of active data in the buffer.
 * @param iob
 * @return size_t
 */
size_t IOBuffer_data_len(const IOBufferRef iob);
void IOBuffer_data_add(IOBufferRef iob, void* p, size_t len);

/**
 * @brief Returns a reference pointer to the start of unused memory space after the last
 * active content in the buffer. This is the start of a memory where more data could be placed.
 * The memory pointed into is owned by the IoBuffer. Do not free
 * @param iob
 * @return void*
 */
void* IOBuffer_space(const IOBufferRef iob);
/**
 * @brief Returns a the length of available space in the buffer after
 * the active data.
 * @param iob
 * @return size_t
 */
size_t IOBuffer_space_len(const IOBufferRef iob);

/**
 * @brief Updates the IoBuffer so that the bytes_used bytes of memory area after the active content
 * is also considered to be active data. The memory addded is not updated as it is expected
 * that data has already been added to that area.
 * @param iob
 * @param bytes_used
 */
void IOBuffer_commit(IOBufferRef iob, size_t bytes_used);
/**
 * @brief Updates the IoBuffer so that the first byte_count bytes of the active data are now
 * considered not active data. IE Increments the start pointer
 * @param iob
 * @param byte_count
 */
void IOBuffer_consume(IOBufferRef iob, size_t byte_count);
/**
 * Only use when absolutely necessary.
 * Frees the memory associated with 'iob' but does not set posize_ter to NULL
 * @param iob
 */
void IOBuffer_free(IOBufferRef iob);
/**
 * @deprecated - dont use
 * @param iob
 */
void IOBuffer_destroy(IOBufferRef iob);
/**
 * @brief Set the buffer back to empty without allocating new memory.
 * @param iob IOBufferRef
 */
void IOBuffer_reset(IOBufferRef iob);
/**
 * @brief Make more space in the buffer. Without changing the overall capacity of the buffer
 * There are two strategies used in iob function:
 * Strategy 1 - move used area to front of allocated memory
 *      When a buffer contains data but no space for new data
 *      iob operation will move the active data to the front of the allocated
 *      memory and relase some memory for space for new data.
 * Strategy 2 - when strategy 1 will not work expand the allocated memory space
 *      by doing a realloc
 */
void IOBuffer_consolidate_space(IOBufferRef iob);
/**
 * @brief Free an IOBuffer and all its associated resources.
 *
 * @Note: The argument is updated to NULL after iob call.
 *
 * @param p IOBufferRef*
 */
//void IOBuffer_dispose(IOBufferRef* p);
bool IOBuffer_empty(IOBufferRef iob);
bool IOBuffer_equal(IOBufferRef a, IOBufferRef b);
/**
 * @brief Get the address of the start of the buffers memory region.
 *
 * @Note: This is a dangerous function as it breaks the integrity of the IOBuffer
 *
 * @param iob IOBufferRef
 * @return void*
 */
void* IOBuffer_memptr(IOBufferRef iob);

char IOBuffer_consume_pop_front(IOBufferRef iob);
void IOBuffer_commit_push_back(IOBufferRef iob, char ch);
void IOBuffer_sprintf(IOBufferRef iob, const char* fmt, ...);
void IOBuffer_append_cstr(IOBufferRef iob, const char* cstr);
void IOBuffer_append_buffer(IOBufferRef iob, const char* buf, size_t len);
/** @} */
#endif