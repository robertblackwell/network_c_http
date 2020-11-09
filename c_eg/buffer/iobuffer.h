#ifndef c_c_eg_buffer_iobuffer_h
#define c_c_eg_buffer_iobuffer_h

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <c_eg/alloc.h>
#define IOBUFFER_DEFAULT_CAPACITY 4*1024
/**
 * IOBuffer is intended for:
 * 1.   reading/writing data from/to say a socket and at the
 *      same time consuming some or all of the read data or producing more write data.
 *      the process would be something like:
 *
 *      bytes_read = read(fd, IOBUffer_readspace(this), IOBUffer_spacelen(this);
 *      IOBuffer_commit(this, bytes_read)
 *      bytes_processed = process_bytes(..... IOBUffer_data(this). IOBuffer_datalen(this))
 *      IOBuffer_consume(this, bytes_processed)
 *
 *      bytes_generated = output_generator( .... IOBuffer_space(this), IOBUffer_spacelen(this))
 *      IOBUffer_commit(this, bytes_generated)
 *      bytes_written = write(fd, IOBuffer_data(this), IOBuffer_datalen(this))
 *      IOBuffer_consume(this, bytes_written)
 *
  */
typedef struct IOBuffer_s {
    void*  mem_p;             // always points to the start of buffer
    char*  char_p;
    int    buffer_capacity;   // always holds the size of the buffer
    void*  buffer_ptr;        // points to the start of unused data in buffer
    int    buffer_length;     // same as capacity
    int    buffer_remaining;  // length of daat no consumed

} IOBuffer, *IOBufferRef;

IOBufferRef IOBuffer_init(IOBufferRef this, int capacity);
IOBufferRef IOBuffer_new_with_capacity(int capacity);
IOBufferRef IOBuffer_new();
void IOBuffer_set_used(IOBufferRef this, int bytes_used);
/**
 * Returns a reference pointer to the start of active data in the buffer.
 * The memory pointed into is owned by the IoBuffer. Do not free
 * \param this
 * \return void*
 */
void* IOBuffer_data(const IOBufferRef this);
/**
 * Returns a the length of active data in the buffer.
 * \param this
 * \return int
 */
int IOBuffer_data_len(const IOBufferRef this);
/**
 * Returns a reference pointer to the start of unused memory space after the last
 * active content in the buffer. This is the start of a memory where more data could be placed.
 * The memory pointed into is owned by the IoBuffer. Do not free
 * \param this
 * \return void*
 */
void* IOBuffer_space(const IOBufferRef this);
/**
 * Returns a the length of available space in the buffer after
 * the active data.
 * \param this
 * \return int
 */
int IOBuffer_space_len(const IOBufferRef this);

/**
 * Updates the IoBuffer so that the bytes_used bytes of memory area after the active content
 * is also considered to be active data. The memory addded is not updated as it is expected
 * that data has already been added to that area.
 * \param this
 * \param bytes_used
 */
void IOBuffer_commit(IOBufferRef this, int bytes_used);
/**
 * Updates the IoBuffer so that the first byte_count bytes of the active data are now
 * considered not active data. IE Increments the start pointer
 * \param this
 * \param byte_count
 */
void IOBuffer_consume(IOBufferRef this, int byte_count);
void IOBuffer_destroy(IOBufferRef this);
void IOBuffer_reset(IOBufferRef this);
void IOBuffer_free(IOBufferRef* p);


#endif