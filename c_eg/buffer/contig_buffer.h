#ifndef lc_c_contig_buffer_t_hpp
#define lc_c_contig_buffer_t_hpp
#include <stddef.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <stdbool.h>

#define CBUFFER_MAX_CAPACITY 10000
#define CBUFFER_MIN_CAPACITY 1000

struct CBuffers;
typedef struct CBuffer_s CBuffer, *CBufferRef;

CBufferRef CBuffer_new();

CBufferRef CBuffer_from_cstring(char* cstr);

void CBuffer_free(CBufferRef cbuf);
/**
 * gets a pointer to the start of the memory slab being managed by the instance
 */
void* CBuffer_data(CBufferRef cbuf);
/**
 * gets the size of used portion of the buffer
*/
size_t CBuffer_size(CBufferRef cbuf);
/**
 * capacity of the buffer - max value of size
*/
size_t CBuffer_capacity(CBufferRef cbuf);
/**
 * returns a pointer to the next available unused position in the buffer
*/
void* CBuffer_next_available(CBufferRef cbuf);
/**
 * Resets the buffer so that it is again an empty buffer
 */
void CBuffer_clear(CBufferRef this);

/**
*append a block of data pointed at by a void* and of given length
*/
void CBuffer_append(CBufferRef cbuf, void* data, size_t len);

/**
*append a block of data given as a cstring with \0 terminator . dont append the \0
*/
void CBuffer_append_cstr(CBufferRef cbuf, char* cstr);
/**
* Force the used size of the buffer
*/
void CBuffer_set_size(CBufferRef cbuf, size_t n);

/**
 * Returns a string that has the same value as the used portion of the buffer
 * This is a reference to an internal string so dont free or change it
 */
char* CBuffer_to_string(CBufferRef cbuf);

void CBuffer_move(CBufferRef dest, CBufferRef src);

/**
 * Detremines if an address value (pointer) is within the address range of the
 * the buffer ie
 *      buffer.dada() < = ptr < buffer.data() + buffer.capacity();
 *  or, should it be
 *      buffer.dada() < = ptr < buffer.data() + buffer.size();
 *
 */
bool CBuffer_contains_voidptr(CBufferRef cbuf, void* ptr);
bool CBuffer_contains_charptr(CBufferRef cbuf, char* ptr);

/**
* These two functions allow for the incomplete consumption of bytes at the leading end of the buffer
*/
typedef struct BufferDescriptor_s {
	void* data;
	size_t len;
} BufferDescriptor;

BufferDescriptor CBuffer_prepare(CBufferRef cbuf);
void CBuffer_consume(CBufferRef cbuf, size_t howmany);

/**
* These functions alow for the addition of bytes at the backend of the buffer while the fron end is being consumed
* NOT YET PROVIDED
*/
#endif
