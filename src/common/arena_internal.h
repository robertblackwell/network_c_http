#ifndef H_chttp_common_arena_internal_H
#define H_chttp_common_arena_internal_H
#include <inttypes.h>
#include <common/alloc.h>
typedef struct Arena_s Arena, *ArenaPtr;
typedef struct MBlock_s MBlock, *MBlockPtr;
typedef struct AllocatedMemory_s
{
    size_t mem_size_bytes;
    uint8_t mem[];
} AllocatedMemory;

struct MBlock_s
{
    MBlockPtr   next_block_ptr;
    size_t      mem_capacity_bytes;
    size_t      mem_next_byte_index;
    union
    {
        uint8_t mem[];
        void*   vmem[];
    };
};
struct Arena_s {
    Allocator allocator;
    MBlockPtr begin;
    MBlockPtr end;
    size_t default_user_capacity;
};
size_t arena_round_up(size_t size_in_bytes);
void arena_fill(void* p, char ch, size_t n);
size_t arena_block_free_space(MBlockPtr block);
uint8_t* arena_block_alloc(MBlockPtr block, size_t alloc_size);
MBlockPtr arena_find_block(ArenaPtr arena, void* needle);
AllocatedMemory* arena_allocated_memory_from_user_ptr(ArenaPtr arena, void* ptr);
void* arena_user_ptr_from_allocation(void* allocation);

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
/*
 * Add a block to the arena that will be big enough to:
 * -    satisfy a user allocation request of size user_capacity_bytes, and
 * -    at least as big as the default block size
 */
MBlockPtr arena_add_block(Arena* arena, size_t user_capacity_bytes);

#endif