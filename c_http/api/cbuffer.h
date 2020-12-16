#ifndef lc_c_contig_buffer_t_hpp
#define lc_c_contig_buffer_t_hpp

#include <stddef.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <stdbool.h>

/**
 * @addtogroup group_cbuffer
 *  A Cbuffer is a contiguous memory allocation that can expand as required.
 *  It maintains:
 *  - the length of the total memory allocation,
 *  - the length of the used part of the memory allocation
 *  - always terminates the used portion with a '\0' which is not counted as part of the used portion
 *    so that the used portion is always a valid cstr
 @{*/
struct Cbuffers;
typedef struct Cbuffer_s *CbufferRef;
// typedef Cbuffer* CbufferRef;

/**
 *  WARNING - THIS FUNCTION ALLOCATES MEMORY
 */
CbufferRef Cbuffer_new();

/**
 *  WARNING - THIS FUNCTION ALLOCATES MEMORY
 */
CbufferRef Cbuffer_from_cstring(const char* cstr);

void Cbuffer_dispose(CbufferRef* cbuf);

/**
 *  @brief Gets a void* pointer to the start of the used portion of memory area
 * 
 *  @param this CbufferRef The buffer, cannot be NULL
 *  @return void* POinter to start of used portion of the managed memory area
 */
void* Cbuffer_data(const CbufferRef this);

/**
 *  @brief Gets a char* pointer to the start of the used portion of memory area,
 *  the first byte after the used portion is always '\0' so that the pointer
 *  returned by this function is a valid c strin.
 * 
 *  @param this CbufferRef The buffer, cannot be NULL
 *  @return void* POinter to start of used portion of the managed memory area
 */
const char* Cbuffer_cstr(const CbufferRef this);

/**
 *  @brief Gets the size of used portion of the buffer
 * 
 *  @param cbuf CbufferRef
 *  @return size_t
 */
size_t Cbuffer_size(const CbufferRef cbuf);

/**
 *  @brief Gets the current capacity of the buffer - max value of size, but
 *
 * @Note: Cbuffer can be extended via realloc so the returned value of
 *  this function is not a constant
 * 
 *  @param cbuf   CbufferRef
 *  @return size_t
 */
size_t Cbuffer_capacity(const CbufferRef cbuf);

/**
 *  @brief Returns a pointer to the next available unused position in the buffer,
 *  which is always the '\0' terminator.
 * 
 *  Should not be used other than by Cbuffer functions.
 * 
 *  @param cbuf  CbufferRef
 *  @return  void*
 */
void* Cbuffer_next_available(const CbufferRef cbuf);

/**
 *  @brief Resets the buffer so that it is again an empty buffer.
 *  Does not release the manage memory area. If it has been expanded by previous
 *  usage pattern the larger memory area will be retained.
 * 
 *  @param this CbufferRef
 */
void Cbuffer_clear(CbufferRef this);

/**
 *  @WARNING - THIS FUNCTION ALLOCATES MEMORY
 * 
 *  @brief Append a block of data pointed at by a void* and of given the  length
 *  to the used portion of the managed memory area. Expand the memory area using realloc
 *  if required.
 * 
 *  The data being appended is copied so that the called retains ownership of and responsibility for
 *  the memory pointed to by the data argument.
 * 
 *  @param cbuf CbufferRef
 *  @param data void*
 *  @param len  size_t
 */
void Cbuffer_append(CbufferRef cbuf, void* data, size_t len);

/**
 *  WARNING - THIS FUNCTION ALLOCATES MEMORY
 * 
 *  @brief Append the data represented by the cstr argument to the used portion of the managed memory area.
 *  Expand the memory area using realloc if required.
 * 
 *  The cstr must be a valid c string terminated by '\0'. The cstr is copied so that the called retains ownership of and responsibility for
 *  the memory pointed to by the cstr argument.
 * 
 *  @param cbuf CbufferRef
 *  @param data void*
 *  @param len  size_t
 */
void Cbuffer_append_cstr(CbufferRef cbuf, const char* cstr);

/**
 *  @brief Force the used size of the buffer to the given value.
 * 
 *  Reserved for Cbuffer internal use.
 */
void Cbuffer_set_size(CbufferRef cbuf, size_t n);

/**
 *  @brief Moves the content of one Cbuffer instance to another by using move semantics.
 *  The internal pointers and counters of src are moved to dest.
 * 
 *  src argument is reset to an empty buffer
 *  @param dest CbufferRef
 *  @param src  CbufferRef
 */
void Cbuffer_move(CbufferRef dest, CbufferRef src);

/**
 *  @brief Detremines if an address value (pointer) is within the address range of the
 *  the buffer ie
 *  ```c
 *       buffer.data() < = ptr < buffer.data() + buffer.capacity();
 *   or, should it be
 *       buffer.data() < = ptr < buffer.data() + buffer.size();
 * ```
 */
bool Cbuffer_contains_voidptr(const CbufferRef cbuf, void* ptr);
bool Cbuffer_contains_charptr(const CbufferRef cbuf, char* ptr);

/**@} */
#endif
