#ifndef c_c_eg_buffer_iobuffer_h
#define c_c_eg_buffer_iobuffer__h

#define _GNU_SOURCE

#include <c_eg/test_helper_types.h>
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
 *      bytes_generated = output_generator( .... IOBuffer_space(this), IOBUffer_spavelen(this))
 *      IOBUffer_commit(this, bytes_generated)
 *      bytes_written = write(fd, IOBuffer_data(this), IOBuffer_datalen(this))
 *      IOBuffer_consume(this, bytes_written)
 *
  */
typedef struct IOBuffer_s {
    char buffer[1000];
    char*  mem_p;             // always points to the start of buffer
    int    buffer_capacity;   // always holds the size of the buffer
    char*  buffer_ptr;        // points to the start of unused data in buffer
    int    buffer_length;     // same as capacity
    int    buffer_remaining;  // length of daat no consumed

} IOBuffer, *IOBufferRef;

IOBufferRef IOBuffer_init(IOBufferRef this, int capacity);
IOBufferRef IOBuffer_new_with_capacity(int capacity);
IOBufferRef IOBuffer_new_with();
void IOBuffer_set_used(IOBufferRef this, int bytes_used);
void* IOBUffer_data(IOBufferRef this);
int IOBuffer_data_len(IOBufferRef this);
void* IOBuffer_space(IOBufferRef this);
int IOBuffer_space_len(IOBuffer this);
void IOBuffer_commit(IOBufferRef this, int bytes_used);
void IOBuffer_consume(IOBufferRef this, int byte_count);

void IOBuffer_consume(IOBufferRef this, int byte_count);
void IOBuffer_destroy(IOBufferRef this);
void IOBuffer_reset(IOBufferRef this);
void IOBuffer_free(IOBufferRef* p);


#endif