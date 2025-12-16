#include "arena.h"
#include "arena_internal.h"
#include <rbl/macros.h>
#include <allocators/alloc.h>

#include <stdlib.h>
#include <string.h>

void arena_fill(void* p, char ch, size_t n)
{
    while(n-- > 0) {
        ((uint8_t*)p)[n] = ch;
    }
}
size_t arena_allocation_size(Arena* arena, void* user_ptr)
{
    return arena_allocated_memory_from_user_ptr(arena, user_ptr)->mem_size_bytes;
}
//
// api starts

#define Arena_TAG "ARENAT"
ArenaPtr arena_create(size_t user_capacity_bytes)
{
    ArenaPtr arena = malloc(sizeof(Arena));
    ARENA_SET_TAG(ARENA_TAG, arena)
    ARENA_SET_END_TAG(ARENA_TAG, arena)
    Allocator* alo = &(arena->allocator);
    ARENA_SET_TAG(Arena_TAG, (alo))
    ARENA_SET_END_TAG(Arena_TAG, (alo))
    ARENA_ASSERT((arena != NULL), "arena create malloc returned NULL");
    ARENA_ASSERT((sizeof(uintptr_t) == sizeof(void*)), "arena create platform invariant failed");
    ARENA_ASSERT((sizeof(uint8_t) == 1),"arena create platform invariant failed");
    ARENA_ASSERT(((user_capacity_bytes % sizeof(uintptr_t)) == 0),"arena create user_capacity_bytes incorrect multiple/alignment");
    arena->begin = NULL;
    arena->end = NULL;
    arena->begin_allocated_memory = NULL;
    arena->default_user_capacity = user_capacity_bytes;
    arena->begin = arena_add_block(arena, user_capacity_bytes);
    arena->end = arena->begin;
    return arena;
}
void arena_destroy(ArenaPtr arena)
{
    ARENA_CHECK_TAG(ARENA_TAG, arena)
    ARENA_CHECK_END_TAG(ARENA_TAG, arena)
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
    ARENA_CHECK_TAG(ARENA_TAG, arena)
    ARENA_CHECK_END_TAG(ARENA_TAG, arena)
    MBlockPtr p = arena->begin;
    while(p != NULL) {
        p->mem_next_byte_index = 0;
        p = p->next_block_ptr;
    }
}
void arena_add_to_allocated_memory_list(Arena* arena, AllocatedMemory* alloc_mem)
{
    alloc_mem->next_allocated_memory_ptr = arena->begin_allocated_memory;
    arena->begin_allocated_memory = alloc_mem;
}
void arena_allocated_memory_init_file_line_number(Arena* arena, AllocatedMemory* alloc_memptr, char* file, int line_number)
{
    alloc_memptr->file_name = file;
    alloc_memptr->line_number = line_number;
}

void* arena_alloc(Arena* arena, size_t user_alloc_size, char* file, int line_number)
{
    ARENA_CHECK_TAG(ARENA_TAG, arena)
    ARENA_CHECK_END_TAG(ARENA_TAG, arena)
    // round up to ensure is multiple of the correct base size usually 8
    user_alloc_size = arena_round_up(user_alloc_size);
    ARENA_ASSERT(((user_alloc_size % sizeof(uintptr_t)) == 0),"alloc_size incorrrect multiple/alignment");
    MBlockPtr pblock = arena->begin;
    while(pblock != NULL) {
        if(arena_block_can_satisfy_alloc(pblock, user_alloc_size)) {
            break;
        }
        pblock = pblock->next_block_ptr;
    }
    if(pblock == NULL) {
        pblock = arena_add_block(arena, user_alloc_size);
    }
    AllocatedMemory* alloc_memptr = arena_block_alloc(arena, pblock, user_alloc_size);
    ARENA_ASSERT((alloc_memptr != NULL), "allocator trying to return NULL");
    ARENA_ALLOCATED_MEMORY_VERIFY(alloc_memptr);

    arena_allocated_memory_init_file_line_number(arena, alloc_memptr, file, line_number);
    arena_add_to_allocated_memory_list(arena, alloc_memptr);

    return arena_allocated_memory_get_user_ptr(alloc_memptr);
}
void* arena_realloc(ArenaPtr arena, void* ptr, size_t alloc_size, char* file, int line_number)
{
    ARENA_CHECK_TAG(ARENA_TAG, arena)
    ARENA_CHECK_END_TAG(ARENA_TAG, arena)
    uint8_t* memptr = ptr;
    size_t old_size = arena_allocation_size(arena, ptr);
    void* newptr = arena_alloc(arena, alloc_size, file, line_number);
    RBL_ASSERT((memptr != NULL), "arena_alloc returned NULL");
    memcpy(newptr, memptr, old_size);
    return newptr;
}
void arena_verify(Arena* arena)
{

}
void arena_verify_allocation(Arena* arena, void* user_ptr)
{
    AllocatedMemory* am = arena_allocated_memory_from_user_ptr(arena, user_ptr);
    AllocatedMemory* p = arena->begin_allocated_memory;
    while(p != NULL) {
        if(p == am) {
            break;
        }
        p = p->next_allocated_memory_ptr;
    }
    assert(p != NULL);
    arena_allocated_memory_verify(p);

}
