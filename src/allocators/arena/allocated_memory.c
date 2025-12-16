#include "arena.h"
#include "arena_internal.h"
#include <rbl/macros.h>
#include <rbl/check_tag.h>
#include <allocators/alloc.h>

#include <stdlib.h>
#include <string.h>

size_t arena_allocated_memory_overhead()
{
#if defined(ARENA_REDZONE_ENABLED)
    return sizeof(AllocatedMemory) + sizeof(RedZone) + rbl_tag_size();
#else
    return sizeof(AllocatedMemory);
#endif
}
size_t arena_allocated_memory_freespace_overhead()
{
#if defined(ARENA_REDZONE_ENABLED)
    return sizeof(AllocatedMemory) + sizeof(RedZone) + offsetof(AllocatedMemory, next_allocated_memory_ptr);
#else
    return sizeof(AllocatedMemory);
#endif
}
size_t arena_allocated_memory_size_for_allocation(size_t allocation_size)
{
    return allocation_size + arena_allocated_memory_overhead();
}

void* arena_allocated_memory_endredzone_ptr(AllocatedMemory* allocated_mem)
{
    void* p = (&(allocated_mem->mem[0])) + allocated_mem->mem_size_bytes;
    return p;
}
void* arena_allocated_memory_endtag_ptr(AllocatedMemory* allocated_mem)
{
    void* p = (&(allocated_mem->mem[0])) + allocated_mem->mem_size_bytes + sizeof(RedZone);
    return p;
}
void arena_allocated_memory_redzone_fill(AllocatedMemory* allocated_mem)
{
    ARENA_CHECK_TAG(ARENA_ALLOCATED_MEMORY_TAG, allocated_mem)
    ARENA_REDZONE_FILL(allocated_mem->redzone_start)
    void* p = (&(allocated_mem->mem[0])) + allocated_mem->mem_size_bytes;
    ARENA_REDZONE_FILL(p)
}
void arena_allocated_memory_init(Arena* arena, AllocatedMemory* allocated_mem, size_t size)
{
    ARENA_SET_TAG(ARENA_ALLOCATED_MEMORY_TAG, allocated_mem)
    ((AllocatedMemory*)allocated_mem)->mem_size_bytes = size;
    allocated_memory_fill_redzones(allocated_mem);
    ARENA_SET_TAG(ARENA_ALLOCATED_MEMORY_TAG, (AllocatedMemory*)arena_allocated_memory_endtag_ptr(allocated_mem))
    arena_fill(&(allocated_mem->mem[0]), 'z', size);
}
AllocatedMemory* arena_allocated_memory_from_user_ptr(ArenaPtr arena, UserMemory ptr)
{
    size_t offset = offsetof(AllocatedMemory, mem);
    AllocatedMemory* amptr = (AllocatedMemory*)((uint8_t*)ptr - offset);
    ARENA_CHECK_TAG(ARENA_ALLOCATED_MEMORY_TAG, amptr)
    return amptr;
}
UserMemory arena_allocated_memory_get_user_ptr(AllocatedMemory* allocated_mem)
{
    ARENA_CHECK_TAG(ARENA_ALLOCATED_MEMORY_TAG, allocated_mem)
    void* p = &(((AllocatedMemory*)allocated_mem)->mem[0]);
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
    ARENA_CHECK_TAG(ARENA_ALLOCATED_MEMORY_TAG, ap)
#if defined(ARENA_REDZONE_ENABLED)
    return &(ap->redzone_start);
#else
    assert(0);
#endif
}
void* allocated_memory_end_redzone_ptr(AllocatedMemory* ap)
{
#if defined(ARENA_REDZONE_ENABLED)
    ARENA_CHECK_TAG(ARENA_ALLOCATED_MEMORY_TAG, ap)
    return (&(ap->mem[0]) + ap->mem_size_bytes);
#else
    assert(0);
#endif
}
void allocated_memory_fill_redzones(AllocatedMemory* allocated_mem)
{
    ARENA_REDZONE_FILL(&(allocated_mem->redzone_start))
    ARENA_REDZONE_FILL(arena_allocated_memory_endredzone_ptr(allocated_mem))
}
void arena_allocated_memory_verify(AllocatedMemory* ap)
{
#if defined(ARENA_REDZONE_ENABLED)
    ARENA_CHECK_TAG(ARENA_ALLOCATED_MEMORY_TAG, ap)
    ARENA_CHECK_TAG(ARENA_ALLOCATED_MEMORY_TAG, (AllocatedMemory*)arena_allocated_memory_endtag_ptr(ap))
    redzone_verify(&(ap->redzone_start));
    void* p = allocated_memory_end_redzone_ptr(ap);
    redzone_verify(allocated_memory_end_redzone_ptr(ap));
#endif
}