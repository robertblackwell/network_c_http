#ifndef lc_c_contig_buffer_t_hpp
#define lc_c_contig_buffer_t_hpp
#include <stddef.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <stdbool.h>
/*
* \ingroup buffers
*
* \brief ContigBuffer provides a template class representing a contiguous expanding block of memory.r
*
* ContigBuffer class wraps a contigous buffer and provides manipulation methods.
* Once constructed the ContigBuffer instance "own" the raw memory.
* ContigBuffer destructor releases the raw memory.
* The template parameter is a strategy for how much to allocate initially and realloc
* when expansion is required.
* S should be 
*   -   default constructable
*   -   void* allocate(std::size_t size)
*   -   std::size_t reallocate_size(std::size_t original_size, std::size_t new_size)
*       like standard realloc should copy the content from the original address to the new address
*   -   void* reallocate(void* p, std::size_t)
*   -   void* free(void* p)
*/
//template<typename S=BufferStrategyInterface>

#define CBUFFER_MAX_CAPACITY 10000
#define CBUFFER_MIN_CAPACITY 1000

struct CBuffers;
typedef struct CBuffer_s CBuffer, *CBufferRef;

// #ifdef XYZ
// /**
// * This struct implements a strategy for growing contiguous buffers
// */
// typedef struct BufferStrategy_s {
//     size_t m_min_size;
//     size_t m_max_size;
// } BufferStrategy, *BufferStrategyRef;


// typedef struct CBuffer_s
// {
//     void*       m_memPtr;     /// points to the start of the memory slab managed by the instance
//     char*       m_cPtr;       /// same as memPtr but makes it easier in debugger to see whats in the buffer
//     size_t      m_length;    ///
//     size_t      m_capacity;  /// the capacity of the buffer, the value used for the malloc call
//     size_t      m_size;      /// size of the currently filled portion of the memory slab
//     BufferStrategy_s* m_strategy;
// } CBuffer, * CBufferRef; 

// #endif
// CBufferRef BodyCBuffer_new();
// CBufferRef HeaderCBuffer_new();
// CBufferRef CBuffer_new_with_strategy();

CBufferRef CBuffer_new();

CBufferRef CBuffer_from_string(char* cstr);

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
void CBuffer_clear();

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
