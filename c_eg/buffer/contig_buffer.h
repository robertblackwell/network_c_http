#ifndef lc_c_contig_buffer_t_hpp
#define lc_c_contig_buffer_t_hpp
#include <stddef.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <stdbool.h>

/// this macro marks all ContigBuffer fuunctions that allocate memory
#define CB_MEMALLOC

///
/// A CBuffer is a contiguous memory allocation that can expand as required.
/// It maintains:
/// - the length of the total memory allocation,
/// - the length of the used part of the memory allocation
/// - always terminates the used portion with a '\0' which is not counted as part of the used portion
///   so that the used portion is always a valid cstr
///
struct CBuffers;
typedef struct CBuffer_s CBuffer, *CBufferRef;

///
/// WARNING - THIS FUNCTION ALLOCATES MEMORY
///
CBufferRef CBuffer_new();

///
/// WARNING - THIS FUNCTION ALLOCATES MEMORY
///
CBufferRef CBuffer_from_cstring(char* cstr);

void CBuffer_free(CBufferRef* cbuf);

///
/// Gets a void* pointer to the start of the used portion of memory area
///
/// \param this CBufferRef The buffer, cannot be NULL
/// \return void* POinter to start of used portion of the managed memory area
///
void* CBuffer_data(CBufferRef this);

///
/// Gets a char* pointer to the start of the used portion of memory area,
/// the first byte after the used portion is always '\0' so that the pointer
/// returned by this function is a valid c strin.
///
/// \param this CBufferRef The buffer, cannot be NULL
/// \return void* POinter to start of used portion of the managed memory area
///
char* CBuffer_cstr(CBufferRef this);

///
/// Gets the size of used portion of the buffer
///
/// \param cbuf
/// \return
size_t CBuffer_size(CBufferRef cbuf);

///
/// Gets the current capacity of the buffer - max value of size, but
/// Note: CBuffer can be extended via realloc so the returned value of
/// this function is not a constant
///
/// \param cbuf
/// \return
size_t CBuffer_capacity(CBufferRef cbuf);

///
/// Returns a pointer to the next available unused position in the buffer,
/// which is always the '\0' terminator.
///
/// Should not be used other than by CBuffer functions.
///
/// \param cbuf
/// \return
///
void* CBuffer_next_available(CBufferRef cbuf);

///
/// Resets the buffer so that it is again an empty buffer.
/// Does not release the manage memory area. If it has been expanded by previous
/// usage pattern the larger memory area will be retained.
///
/// \param this CBuffer
void CBuffer_clear(CBufferRef this);

///
/// WARNING - THIS FUNCTION ALLOCATES MEMORY
///
/// Append a block of data pointed at by a void* and of given the  length
/// to the used portion of the managed memory area. Expand the memory area using realloc
/// if required.
///
/// The data being appended is copied so that the called retains ownership of and responsibility for
/// the memory pointed to by the data argument.
///
/// \param cbuf
/// \param data
/// \param len
///
void CBuffer_append(CBufferRef cbuf, void* data, size_t len);

///
/// WARNING - THIS FUNCTION ALLOCATES MEMORY
///
/// Append the data represented by the cstr argument to the used portion of the managed memory area.
/// Expand the memory area using realloc if required.
///
/// The cstr must be a valid c string terminated by '\0'. The cstr is copied so that the called retains ownership of and responsibility for
/// the memory pointed to by the cstr argument.
///
/// \param cbuf
/// \param data
/// \param len
///
void CBuffer_append_cstr(CBufferRef cbuf, char* cstr);

///
/// Force the used size of the buffer to the given value.
///
/// Reserved for CBuffer internal use.
///
void CBuffer_set_size(CBufferRef cbuf, size_t n);

///
/// Moves the content of one CBuffer instance to another by using move semantics.
/// The internal pointers and counters of src are moved to dest.
///
/// src argument is reset to an empty buffer
/// \param dest
/// \param src
///
void CBuffer_move(CBufferRef dest, CBufferRef src);

///
/// Detremines if an address value (pointer) is within the address range of the
/// the buffer ie
///      buffer.dada() < = ptr < buffer.data() + buffer.capacity();
///  or, should it be
///      buffer.dada() < = ptr < buffer.data() + buffer.size();
///
///
bool CBuffer_contains_voidptr(CBufferRef cbuf, void* ptr);
bool CBuffer_contains_charptr(CBufferRef cbuf, char* ptr);


#endif
