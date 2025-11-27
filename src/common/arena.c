#include "arena.h"
#include "arena_internal.h"
#include <common/alloc.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
void* api_arena_allocate(Allocator* allocator, size_t size)
{
    return arena_alloc((Arena*)allocator, size);
}
void* api_arena_reallocate(Allocator* allocator, void* ptr, size_t size)
{
    return arena_realloc((Arena*)allocator , ptr, size);
}
void  api_arena_deallocate(Allocator* allocator, void* ptr)
{
}
void api_arena_reset(Allocator* allocator)
{
    arena_reset((Arena*)allocator);
}
void api_arena_destroy(Allocator* allocator)
{
    arena_destroy((Arena*)allocator);
}
Allocator* arena_allocator_create(size_t capacity)
{
    Allocator* allocator = (Allocator*)arena_create(capacity);
    allocator->allocate = &api_arena_allocate;
    allocator->reallocate = &api_arena_reallocate;
    allocator->deallocate = NULL;
    allocator->reset = &api_arena_reset;
    return allocator;
}
void arena_fill(void* p, char ch, size_t n)
{
    while(n-- > 0) {
        ((uint8_t*)p)[n] = ch;
    }
}
size_t arena_block_free_space(MBlockPtr block)
{
    return block->mem_capacity_bytes - block->mem_next_byte_index;
}
size_t arena_allocation_size(Arena* arena, void* user_ptr)
{
    return arena_allocated_memory_from_user_ptr(arena, user_ptr)->mem_size_bytes;
}
uint8_t* arena_block_alloc(MBlockPtr block, size_t user_alloc_size)
{
    user_alloc_size = arena_round_up(user_alloc_size);
    size_t alloc_size = user_alloc_size + sizeof(AllocatedMemory);
    assert(arena_block_free_space(block) >= alloc_size);
    uint8_t* memptr = &(block->mem[0]) + block->mem_next_byte_index;
    AllocatedMemory* allocaptr = (AllocatedMemory*)memptr;
    block->mem_next_byte_index += alloc_size;
    arena_fill(memptr, 'z', alloc_size);
    allocaptr->mem_size_bytes = user_alloc_size;
    void* result = arena_user_ptr_from_allocation(memptr);
    return result;
}

size_t arena_round_up(size_t size_in_bytes)
{
    return ((size_in_bytes % sizeof(uintptr_t)) == 0) ? size_in_bytes: sizeof(uintptr_t)*(1+(size_in_bytes / sizeof(uintptr_t)));
}
MBlockPtr arena_add_block(Arena* arena, size_t user_capacity_bytes)
{
    size_t capacity_bytes;
    if(user_capacity_bytes == 0) {
        capacity_bytes = arena->default_user_capacity + sizeof(AllocatedMemory);
    } else {
        capacity_bytes = (user_capacity_bytes <= arena->default_user_capacity)
        ? arena->default_user_capacity + sizeof(AllocatedMemory)
        : 2*(user_capacity_bytes+sizeof(AllocatedMemory));
    }
    size_t block_size = sizeof(MBlock) + (capacity_bytes * sizeof(uint8_t));
    MBlockPtr bp = malloc(block_size);
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
MBlockPtr arena_find_block(ArenaPtr arena, void* needle)
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
AllocatedMemory* arena_allocated_memory_from_user_ptr(ArenaPtr arena, void* ptr)
{
    size_t offset = offsetof(AllocatedMemory, mem);
    return (AllocatedMemory*)((uint8_t*)ptr - offset);
}
void* arena_user_ptr_from_allocation(void* allocation)
{
    void* p = &(((AllocatedMemory*)allocation)->mem[0]);
    return p;
}
//
// api starts

#define Arena_TAG "ARENAT"
ArenaPtr arena_create(size_t user_capacity_bytes)
{
    ArenaPtr arena = malloc(sizeof(Arena));
    Allocator* alo = &(arena->allocator);
    RBL_SET_TAG(Arena_TAG, (alo))
    RBL_SET_END_TAG(Arena_TAG, (alo))
    assert(arena != NULL);
    assert(sizeof(uintptr_t) == sizeof(void*));
    assert(sizeof(uint8_t) == 1);
    assert((user_capacity_bytes % sizeof(uintptr_t)) == 0);
    // assert(user_capacity_bytes >= ARENA_DEFAULT_CAPACITY);
    arena->begin = NULL;
    arena->end = NULL;
    arena->default_user_capacity = user_capacity_bytes;
    arena->begin = arena_add_block(arena, user_capacity_bytes);
    arena->end = arena->begin;
    return arena;
}
void arena_destroy(ArenaPtr arena)
{
    MBlockPtr p = arena->begin;
    while(p != NULL) {
        MBlock* tmp = p->next_block_ptr;
        free(p);
        p = tmp;
    }
    free(arena);
}
void arena_reset(ArenaPtr arena)
{
    MBlockPtr p = arena->begin;
    while(p != NULL) {
        p->mem_next_byte_index = 0;
        p = p->next_block_ptr;
    }
}
void* arena_alloc(Arena* arena, size_t alloc_size)
{
    // assert(alloc_size < ARENA_DEFAULT_CAPACITY);
    alloc_size = arena_round_up(alloc_size);
    assert((alloc_size % sizeof(uintptr_t)) == 0);
    MBlockPtr p = arena->begin;
    while(p != NULL) {
        if(alloc_size <= arena_block_free_space(p)) {
            void* memptr = arena_block_alloc(p, alloc_size);
            return memptr;
        }
        p = p->next_block_ptr;
    }
    // here because could not find space - new block
    MBlockPtr newptr = arena_add_block(arena, alloc_size);
    void* memptr = arena_block_alloc(newptr, alloc_size);
    assert(memptr != NULL);
    return memptr;
}
void* arena_realloc(ArenaPtr arena, void* ptr, size_t alloc_size)
{
    uint8_t* memptr = ptr;
    size_t old_size = arena_allocation_size(arena, ptr);
    void* newptr = arena_alloc(arena, alloc_size);
    assert(memptr != NULL);
    memcpy(newptr, memptr, old_size);
    return newptr;
}
