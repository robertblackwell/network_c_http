#include "arena.h"
#include "arena_internal.h"
#include <rbl/check_tag.h>
#include <rbl/macros.h>
#include <allocators/alloc.h>

#include <stdlib.h>
#include <string.h>
size_t arena_round_up(size_t size_in_bytes)
{
    return ((size_in_bytes % sizeof(uintptr_t)) == 0) ? size_in_bytes: sizeof(uintptr_t)*(1+(size_in_bytes / sizeof(uintptr_t)));
}

// calculates the address of the first byte of freespace within a block
#define BlockFreespacePtr(block) (&(block->mem[0]) + block->mem_next_byte_index);

size_t arena_block_free_space(MBlockPtr block)
{
    return block->mem_capacity_bytes - block->mem_next_byte_index;
}
size_t arena_block_size(size_t mem_capacity)
{
    return sizeof(MBlock) + mem_capacity + rbl_tag_size();
}
AllocatedMemory* arena_block_alloc(Arena* arena, MBlockPtr block, size_t user_alloc_size)
{
    user_alloc_size = arena_round_up(user_alloc_size);
    size_t alloc_mem_size = arena_allocated_memory_size_for_allocation(user_alloc_size);
    size_t xx = arena_block_free_space(block);
    RBL_ASSERT((arena_block_free_space(block) >= alloc_mem_size), "arena block allocate invariant failed");
    // this is pointer to start of free space
    // uint8_t* memptr = &(block->mem[0]) + block->mem_next_byte_index;
    void* memptr = BlockFreespacePtr(block)
    AllocatedMemory* alloc_ptr = (AllocatedMemory*)memptr;
    block->mem_next_byte_index += alloc_mem_size;
    void* tmp = &(block->mem[0]) + block->mem_next_byte_index;
    block->mem_next_byte_ptr = tmp;
    arena_allocated_memory_init(arena, alloc_ptr, user_alloc_size);
    return alloc_ptr;
}

MBlockPtr arena_add_block(Arena* arena, size_t user_capacity_bytes)
{
    size_t capacity_bytes;
    if(user_capacity_bytes == 0) {
        capacity_bytes = arena->default_user_capacity;
    } else {
        capacity_bytes = (user_capacity_bytes <= arena->default_user_capacity)
        ? arena->default_user_capacity
        : 2 * (arena_block_require_freespace(user_capacity_bytes));
    }
    // size_t block_size2 = sizeof(MBlock) + (capacity_bytes * sizeof(uint8_t));
    size_t block_size = arena_block_size(capacity_bytes * sizeof(uint8_t));
    // if(block_size != block_size2)
    //     assert(block_size == block_size2);
    MBlockPtr bp = malloc(block_size);
    ARENA_SET_TAG(ARENA_BLOCK_TAG, bp);
    bp->mem_capacity_bytes = capacity_bytes;
    bp->mem_next_byte_index = 0;
    bp->next_block_ptr = NULL;
    if(arena->begin == NULL) {
        arena->begin = bp;
        arena->end = bp;
    } else {
        arena->end->next_block_ptr = bp;
        arena->end = bp;
    }
    return bp;
}
MBlockPtr arena_find_block(ArenaPtr arena, UserMemory needle)
{
    MBlockPtr p = arena->begin;
    uint8_t* pn = (uint8_t*)arena_allocated_memory_from_user_ptr(arena, needle);
    while(p != NULL) {
        if((pn >= &(p->mem[0])) && (pn < &(p->mem[p->mem_capacity_bytes]))) {
            return p;
        }
        p = p->next_block_ptr;
    }
    return NULL;
}
#if 0
void arena_allocated_memory_redzone_fill(AllocatedMemory* allocated_mem)
{
    ARENA_REDZONE_FILL(allocated_mem->redzone_start)
    void* p = (&(allocated_mem->mem[0])) + allocated_mem->mem_size_bytes;
    ARENA_REDZONE_FILL(p)
}
void arena_allocated_memory_init(AllocatedMemory* allocated_mem, size_t size)
{
    ((AllocatedMemory*)allocated_mem)->mem_size_bytes = size;
    ARENA_REDZONE_FILL(allocated_mem->redzone_start)
    void* p = (&(allocated_mem->mem[0])) + allocated_mem->mem_size_bytes;
    ARENA_REDZONE_FILL(p)
    arena_fill(&(allocated_mem->mem[0]), 'z', size);
}
AllocatedMemory* arena_allocated_memory_from_user_ptr(ArenaPtr arena, UserMemory ptr)
{
    size_t offset = offsetof(AllocatedMemory, mem);
    AllocatedMemory* amem = (AllocatedMemory*)((uint8_t*)ptr - offset);
    // ASSERT_REDZONE(amem->redzone_begin, ARENA_REDZONE_CHAR);
    // ASSERT_REDZONE(&(amem->mem)+amem->mem_size_bytes, ARENA_REDZONE_CHAR);
    return (AllocatedMemory*)((uint8_t*)ptr - offset);
}
UserMemory arena_user_ptr_from_allocated_memory(AllocatedMemory* allocation)
{
    void* p = &(((AllocatedMemory*)allocation)->mem[0]);
    return p;
}
#endif
void arena_block_validate(MBlockPtr p)
{

}
MBlockPtr arena_block_next(MBlockPtr block)
{
    MBlockPtr memptr = (MBlockPtr)&(block->mem[0]) + block->mem_next_byte_index;
    arena_block_validate(memptr);
    return memptr;
}
size_t arena_block_require_freespace(size_t user_alloc_size)
{
    size_t xx = arena_allocated_memory_size_for_allocation(user_alloc_size);
#if defined(ARENA_REDZONE_ENABLED)
    size_t required_freespace_size = user_alloc_size + arena_allocated_memory_overhead();
    assert(xx == required_freespace_size);
    // size_t required_freespace_size = user_alloc_size + sizeof(AllocatedMemory) + sizeof(RedZone);
#else
    size_t required_freespace_size = user_alloc_size  + arena_allocated_memory_overhead();
#endif
    return required_freespace_size;
}
bool arena_block_can_satisfy_alloc(MBlockPtr block, size_t user_alloc_size)
{
    user_alloc_size = arena_round_up(user_alloc_size);
    RBL_ASSERT(((user_alloc_size % sizeof(uintptr_t)) == 0),"alloc_size incorrrect multiple/alignment");
    size_t min_freespace_required = arena_block_require_freespace(user_alloc_size);
    return (min_freespace_required <= arena_block_free_space(block));
}
