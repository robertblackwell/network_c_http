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
 *      bytes_read = read(fd, IOBuffer_space(this), IOBUffer_space_len(this);
 *      IOBuffer_commit(this, bytes_read)
 *      bytes_processed = process_bytes(..... IOBuffer_data(this). IOBuffer_data_len(this))
 *      IOBuffer_consume(this, bytes_processed)
 *```
 *
 * ```c
 *      bytes_generated = output_generator( .... IOBuffer_space(this), IOBUffer_spacelen(this))
 *      IOBUffer_commit(this, bytes_generated)
 *      bytes_written = write(fd, IOBuffer_data(this), IOBuffer_datalen(this))
 *      IOBuffer_consume(this, bytes_written)
 *```
 *
 * NOTE: IOBuffers never expand - they can be made to have any capacity needed at creation time, there after
 * they cannot expand. A consequence of this is that there re no "append" style methods.
 *
 * It would be dangerous to allow a buffer to expand (and the address of the underlying memory possibly change)
 * while the same buffer was being used for IO
 *
 * @{*/

/**
 * @brief IOBuffer as an opaque object.
 */
typedef struct IOBuffer_s IOBuffer, *IOBufferRef;

IOBufferRef IOBuffer_init(IOBufferRef this, int capacity);
/**
 * @brief Create a new IOBuffer with at least the requested capacity in bytes.
 *
 * @param capacity int The capacity of the new IOBuffer in bytes.
 * @return IOBufferRef
 */
IOBufferRef IOBuffer_new_with_capacity(int capacity);

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
 *      B) undefined - to do this we will need a new Cbuffer method called Cbuffer_steal_content
 *          needs more thinking about
 *
 */
IOBufferRef IOBuffer_from_cbuffer(CbufferRef cbuf);

/**
 * @brief Makes an IOBUffer from the a pointer and length by COPY
 * @param buf char* pointing to start of data to put in IOBUffer
 * @param len int   length of data
 * @return IOBufferRef
 */
IOBufferRef IOBuffer_from_buf(char* buf, int len);

/**
 * @brief Makes an IOBuffer from a c-string by COPY
 * @param cbuf  CbufferRef
 * @return IOBufferRef
 */
IOBufferRef IOBuffer_from_cstring(char* cstr);
/**
 * @brief Returns a c string ref to internal data
 * @param this IOBuffer
 * @return c string Weak reference do not free
 */
const char* IOBuffer_cstr(IOBufferRef this);
/**
 * @brief Duplicate an IOBuffer including copying the content to a new memory allocation.
 * @param this IOBUfferRef
 * @return IOBufferRef
 */
IOBufferRef IOBuffer_dup(IOBufferRef this);
void IOBuffer_set_used(IOBufferRef this, int bytes_used);
/**
 * @brief Returns a reference pointer to the start of active data in the buffer.
 * The memory pointed into is owned by the IoBuffer. Do not free
 * @param this
 * @return void*
 */
void* IOBuffer_data(const IOBufferRef this);
/**
 * @brief Returns a the length of active data in the buffer.
 * @param this
 * @return int
 */
int IOBuffer_data_len(const IOBufferRef this);
void IOBuffer_data_add(IOBufferRef this, void* p, int len);

/**
 * @brief Returns a reference pointer to the start of unused memory space after the last
 * active content in the buffer. This is the start of a memory where more data could be placed.
 * The memory pointed into is owned by the IoBuffer. Do not free
 * @param this
 * @return void*
 */
void* IOBuffer_space(const IOBufferRef this);
/**
 * @brief Returns a the length of available space in the buffer after
 * the active data.
 * @param this
 * @return int
 */
int IOBuffer_space_len(const IOBufferRef this);

/**
 * @brief Updates the IoBuffer so that the bytes_used bytes of memory area after the active content
 * is also considered to be active data. The memory addded is not updated as it is expected
 * that data has already been added to that area.
 * @param this
 * @param bytes_used
 */
void IOBuffer_commit(IOBufferRef this, int bytes_used);
/**
 * @brief Updates the IoBuffer so that the first byte_count bytes of the active data are now
 * considered not active data. IE Increments the start pointer
 * @param this
 * @param byte_count
 */
void IOBuffer_consume(IOBufferRef this, int byte_count);
/**
 * Only use when absolutely necessary.
 * Frees the memory associated with 'this' but does not set pointer to NULL
 * @param this
 */
void IOBuffer_free(IOBufferRef this);
/**
 * @deprecated - dont use
 * @param this
 */
void IOBuffer_destroy(IOBufferRef this);
/**
 * @brief Set the buffer back to empty without allocating new memory.
 * @param this IOBufferRef
 */
void IOBuffer_reset(IOBufferRef this);
/**
 * @brief Make more space in the buffer. Without changing the overall capacity of the buffer
 * There are two strategies used in this function:
 * Strategy 1 - move used area to front of allocated memory
 *      When a buffer contains data but no space for new data
 *      this operation will move the active data to the front of the allocated
 *      memory and relase some memory for space for new data.
 * Strategy 2 - when strategy 1 will not work expand the allocated memory space
 *      by doing a realloc
 */
void IOBuffer_consolidate_space(IOBufferRef this);
/**
 * @brief Free an IOBuffer and all its associated resources.
 *
 * @Note: The argument is updated to NULL after this call.
 *
 * @param p IOBufferRef*
 */
//void IOBuffer_dispose(IOBufferRef* p);
bool IOBuffer_empty(IOBufferRef this);
bool IOBuffer_equal(IOBufferRef a, IOBufferRef b);
/**
 * @brief Get the address of the start of the buffers memory region.
 *
 * @Note: This is a dangerous function as it breaks the integrity of the IOBuffer
 *
 * @param this IOBufferRef
 * @return void*
 */
void* IOBuffer_memptr(IOBufferRef this);

char IOBuffer_consume_pop_front(IOBufferRef this);
void IOBuffer_commit_push_back(IOBufferRef this, char ch);
void IOBuffer_sprintf(IOBufferRef iob, const char* fmt, ...);
/** @} */
#endif