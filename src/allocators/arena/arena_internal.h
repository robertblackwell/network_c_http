#ifndef H_chttp_common_arena_internal_structs_H
#define H_chttp_common_arena_internal_structs_H

#include <inttypes.h>
#include <allocators/alloc.h>
typedef uint64_t RedZone;
typedef struct Arena_s Arena, *ArenaPtr;
typedef struct MBlock_s MBlock, *MBlockPtr;
typedef struct AllocatedMemory_s AllocatedMemory;
typedef void* UserMemory;
/******************************************************************************************
* Functions that are internal but used in multuple files and for testing
*******************************************************************************************/
typedef struct AllocatedMemory_s AllocatedMemory;
void arena_verify_allocation(Arena* arena, void* user_ptr);
size_t arena_round_up(size_t size_in_bytes);
void arena_fill(void* p, char ch, size_t n);
size_t arena_allocation_size(Arena* arena, void* user_ptr);

void arena_block_validate(MBlockPtr p);
size_t arena_allocated_memory_size_for_allocation(size_t allocation_size);
size_t arena_allocated_memory_overhead();
size_t arena_allocate_memory_freespace_overhead();
size_t arena_block_free_space(MBlockPtr block);
void arena_allocated_memory_verify(AllocatedMemory* ap);
void allocated_memory_fill_redzones(AllocatedMemory* allocated_mem);

/**
 * Find the block containing a user pointer, that is the block from which
 * the associated AllocatedMemory was allocated.
 */
MBlockPtr arena_find_block(ArenaPtr arena, UserMemory needle);
/**
 *
 * @param block pointer to a block with enough freespace to satisfy a users request for memory of size alloc_size
 * @param alloc_size The size of memory the caller requires.
 * @return a pointer to AllocatedMemory where the mem property is at least as big as alloc_size that the caller may use.
 * @error fatal error is the block does not have enough space. The arena code that calls this function should
 * ensure the block has enough freespace before calling this function
 */
AllocatedMemory* arena_block_alloc(Arena* arena, MBlockPtr block, size_t alloc_size);
/*
 * Initialize a instance of AllocatedMemory
 */
void arena_allocated_memory_init(Arena* arena, AllocatedMemory* allocated_mem, size_t size);

/**
 * The function arena_block_alloc returns a pointer to the start useable memory (&allocated_memory.mem)inside
 * an instance of AllocatedMemory , that pointer
 * is not a pointer to the memory that is available for use by the user/caller.
 *
 * This function converts a block pointer to the memory inside the block that is available for use by the caller.
 *
 * @param allocation void* a pointer to a block returned by arena_block_allocate.
 * @return void* pointer to the memory that can be used by the caller
 */
UserMemory arena_allocated_memory_get_user_ptr(AllocatedMemory* allocation);
/**
 * This function is the inverse of arena_user_ptr_from_allocation.
 * @param arena ArenaPtr - an arena memory pool
 * @param ptr void* a pointer to the user memory inside an allocated block
 * @return pointer to the allocated block in which the ptr resides.
 */
AllocatedMemory* arena_allocated_memory_from_user_ptr(ArenaPtr arena, void* ptr);
/**
 * Give the size of the allocation the user requires compute the minimum freespace a block
 * would require to be able to provide that user allocation and hence a call to
 * arena_block_allocate(user_alloc_size) would not fail
 */
size_t arena_block_require_freespace(size_t user_alloc_size);
/**
 *  Will return true if the given block has enough free space to satisfy a user
 *  allocation request for "user_alloc_size" bytes
 */
bool arena_block_can_satisfy_alloc(MBlockPtr block, size_t user_alloc_size);
/**
 * Add a block to the arena that will be big enough to:
 * -    satisfy a user allocation request of size user_capacity_bytes, and
 * -    at least as big as the default block size
 */
MBlockPtr arena_add_block(Arena* arena, size_t user_capacity_bytes);

//////////////////////////////////////////////////////////////////////////////////////////////////////
/// macros - mostly used to implement debug mode where the code checks for incosistencies and
/// provides functions and macros for diagnosing problems
//////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * ARENA_DEBUG turns on all the debugging and error trapping code
 */
// #define ARENA_DEBUG
#if defined(ARENA_DEBUG)
#define ARENA_REDZONE_ENABLED
#else
#undef  ARENA_REDZONE_ENABLED
#endif

#define ARENA_REDZONE_CHAR ((char)'\xcd')
#define ARENA_ALLOCATED_MEMORY_TAG "aralmem"
#define ARENA_BLOCK_TAG "arblktag"
#define ARENA_TAG "ARENA"

#if defined(ARENA_REDZONE_ENABLED)
#warning ARENA IN DEBUG MODE
#define ARENA_DECLARE_TAG RBL_DECLARE_TAG
#define ARENA_DECLARE_END_TAG   RBL_DECLARE_END_TAG
#define ARENA_SET_TAG(p, tag)   RBL_SET_TAG(p, tag)
#define ARENA_CHECK_TAG(p, tag) RBL_CHECK_TAG(p, tag)
#define ARENA_SET_END_TAG(p, tag)   RBL_SET_END_TAG(p, tag)
#define ARENA_CHECK_END_TAG(p, tag) RBL_CHECK_END_TAG(p, tag)

#define ARENA_REDSZONE_SIZE 8
#define ARENA_REDZONE_DECLARE_START  uint64_t redzone_start;
#define ARENA_REDZONE_DECLARE_END    union{char[8]; uint64_t} redzone_start;

#define ARENA_REDZONE_FILL(p) memset((void*)p, ARENA_REDZONE_CHAR, 8);
#define ARENA_ALLOCATED_MEMORY_VERIFY(p) arena_allocated_memory_verify(p);
#define ARENA_ASSERT(c, m) RBL_ASSERT(c, m)

#else
#warning ARENA IN NO DEBUG MODE
#define ARENA_DECLARE_TAG
#define ARENA_DECLARE_END_TAG
#define ARENA_SET_TAG(p, tag)
#define ARENA_SET_END_TAG(p, tag)
#define ARENA_CHECK_TAG(p, tag)
#define ARENA_CHECK_END_TAG(p, tag)

#define ARENA_REDSZONE_SIZE 8
#define ARENA_REDZONE_DECLARE_START
#define ARENA_REDZONE_DECLARE_END
#define ARENA_REDZONE_FILL(p)
#define ARENA_ALLOCATED_MEMORY_VERIFY(p)
#define ARENA_ASSERT(c, m)
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// struct definitions - the order is a bit unusual as some of the macros are needed for the structs
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This struct is the unit of allocation. They are variable size
// There are (conditionally) guards or redzones around the memory the caller gets access to.
// This allows the arena to check for writing out-of-bounds
typedef struct AllocatedMemory_s
{
    ARENA_DECLARE_TAG;
    AllocatedMemory* next_allocated_memory_ptr;
    size_t mem_size_bytes;
    char*  file_name;
    int    line_number;
    ARENA_REDZONE_DECLARE_START //this will be empty when redzones are not enabled
    uint8_t mem[];
    //ARENA_REDZONE_DECLARE_AFTER - there is a redzone after the useable memory - but it makes no sense to declare it in the struct
    //ARENA_DECLARE_END_TAG - this is at a variable position and needs to be calculated at runtime
} AllocatedMemory;


// This is a pointer to the field AllocatedMemory mem[0] in an instance of AllocatedMemory that was used to satisfy
// a callers request for memory.
typedef void* UserMemory;

struct MBlock_s
{
    ARENA_DECLARE_TAG;
    MBlockPtr   next_block_ptr;
    size_t      mem_capacity_bytes;
    size_t      mem_next_byte_index;
    void*       mem_next_byte_ptr;
    uint8_t     mem[];
    //ARENA_DECLARE_END_TAG - this is at a variable position and needs to be calculated at runtime
};
struct Arena_s {
    ARENA_DECLARE_TAG;
    Allocator allocator;
    MBlockPtr begin;
    MBlockPtr end;
    AllocatedMemory* begin_allocated_memory;
    size_t default_user_capacity;
    ARENA_DECLARE_END_TAG;
};


#endif