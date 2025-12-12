#include "arena.h"
#include "arena_internal.h"
#include <rbl/macros.h>
#include <common/alloc.h>

#include <stdlib.h>
#include <string.h>
#if 0
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
    allocator->destroy = &api_arena_destroy;
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
// calculates the address of the first byte of freespace within a block
#define BlockFreespacePtr(block) (&(block->mem[0]) + block->mem_next_byte_index);
#define AllocatedMemoryInit(amptr, amsize) do{ \
        ((AllocatedMemory*)amptr)->mem_size_bytes = amsize; \
        arena_fill(memptr, 'z', alloc_size); \
    }while(0);
UserMemory arena_block_alloc(MBlockPtr block, size_t user_alloc_size)
{
    user_alloc_size = arena_round_up(user_alloc_size);
    size_t alloc_mem_size = user_alloc_size + sizeof(AllocatedMemory);
    size_t xx = arena_block_free_space(block);
    RBL_ASSERT((arena_block_free_space(block) >= alloc_mem_size), "arena block allocate invariant failed");
    // this is pointer to start of free space
    // uint8_t* memptr = &(block->mem[0]) + block->mem_next_byte_index;
    void* memptr = BlockFreespacePtr(block)
    AllocatedMemory* alloc_ptr = (AllocatedMemory*)memptr;
    block->mem_next_byte_index += alloc_mem_size;
#if 1
    arena_allocated_memory_init(alloc_ptr, user_alloc_size);
#else
    arena_fill(&(alloc_ptr->mem[0]), 'z', user_alloc_size);
    alloc_ptr->mem_size_bytes = user_alloc_size;
#endif
    void* result = arena_user_ptr_from_allocation((AllocatedMemory*)memptr);
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
        : 2 * (arena_block_require_freespace(user_capacity_bytes));
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
#endif
size_t arena_allocated_memory_overhead()
{
#if defined(ARENA_REDZONE_ENABLED)
    return sizeof(AllocatedMemory) + sizeof(RedZone);
#else
    return sizeof(AllocatedMemory);
#endif
}
size_t arena_allocated_memory_freespace_overhead()
{
#if defined(ARENA_REDZONE_ENABLED)
    return sizeof(AllocatedMemory) + sizeof(RedZone);
#else
    return sizeof(AllocatedMemory);
#endif
}

void arena_allocated_memory_redzone_fill(AllocatedMemory* allocated_mem)
{
    ARENA_REDZONE_FILL(allocated_mem->redzone_start)
    void* p = (&(allocated_mem->mem[0])) + allocated_mem->mem_size_bytes;
    ARENA_REDZONE_FILL(p)
}
void arena_allocated_memory_init(AllocatedMemory* allocated_mem, size_t size)
{
    ((AllocatedMemory*)allocated_mem)->mem_size_bytes = size;
    ARENA_REDZONE_FILL(&(allocated_mem->redzone_start))
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
UserMemory arena_allocated_memory_get_user_ptr(AllocatedMemory* allocation)
{
    void* p = &(((AllocatedMemory*)allocation)->mem[0]);
    return p;
}
void redzone_verify(void* p)
{
    char* q = (char*)p;
    for(int i = 0; i < 8; i++) {
        if(*q != ARENA_REDZONE_CHAR) {
            assert(0);
        }
        q++;
    }

}
void* allocated_memory_start_redzone_ptr(AllocatedMemory* ap)
{
#if defined(ARENA_REDZONE_ENABLED)
    return &(ap->redzone_start);
#else
    assert(0);
#endif
}
void* allocated_memory_end_redzone_ptr(AllocatedMemory* ap)
{
    return (&(ap->mem[0]) + ap->mem_size_bytes);
}
void arena_allocated_memory_redzone_verify(AllocatedMemory* ap, const char* file, int line_number)
{
#if defined(ARENA_REDZONE_ENABLED)
    redzone_verify(&(ap->redzone_start));
    redzone_verify(allocated_memory_end_redzone_ptr(ap));
#endif
}

#if 0
void arena_block_validate(MBlockPtr p)
{

}
MBlockPtr arena_block_next(MBlockPtr block)
{
    MBlockPtr memptr = (MBlockPtr)&(block->mem[0]) + block->mem_next_byte_index;
    arena_block_validate(memptr);
    return memptr;
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
    RBL_ASSERT((arena != NULL), "arena create malloc returned NULL");
    RBL_ASSERT((sizeof(uintptr_t) == sizeof(void*)), "arena create platform invariant failed");
    RBL_ASSERT((sizeof(uint8_t) == 1),"arena create platform invariant failed");
    RBL_ASSERT(((user_capacity_bytes % sizeof(uintptr_t)) == 0),"arena create user_capacity_bytes incorrect multiple/alignment");
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
size_t arena_block_require_freespace(size_t user_alloc_size)
{
    size_t required_freespace_size = user_alloc_size + sizeof(AllocatedMemory);
    return required_freespace_size;
}
bool arena_block_can_satisfy_alloc(MBlockPtr block, size_t user_alloc_size)
{
    user_alloc_size = arena_round_up(user_alloc_size);
    RBL_ASSERT(((user_alloc_size % sizeof(uintptr_t)) == 0),"alloc_size incorrrect multiple/alignment");
    size_t min_freespace_required = arena_block_require_freespace(user_alloc_size);
    return (min_freespace_required <= arena_block_free_space(block));
}

void* arena_alloc(Arena* arena, size_t user_alloc_size)
{
    // round up to ensure is multiple of the correct base size usually 8
    user_alloc_size = arena_round_up(user_alloc_size);
    RBL_ASSERT(((user_alloc_size % sizeof(uintptr_t)) == 0),"alloc_size incorrrect multiple/alignment");
    MBlockPtr p = arena->begin;
    while(p != NULL) {
        if(arena_block_can_satisfy_alloc(p, user_alloc_size)) {
            void* memptr = arena_block_alloc(p, user_alloc_size);
            return memptr;
        }
        p = p->next_block_ptr;
    }
    // here because could not find space - new block
    MBlockPtr newptr = arena_add_block(arena, user_alloc_size);
    void* memptr = arena_block_alloc(newptr, user_alloc_size);
    RBL_ASSERT((memptr != NULL), "allocator trying to return NULL");
    return memptr;
}
void* arena_realloc(ArenaPtr arena, void* ptr, size_t alloc_size)
{
    uint8_t* memptr = ptr;
    size_t old_size = arena_allocation_size(arena, ptr);
    void* newptr = arena_alloc(arena, alloc_size);
    RBL_ASSERT((memptr != NULL), "arena_alloc returned NULL");
    memcpy(newptr, memptr, old_size);
    return newptr;
}
#endif