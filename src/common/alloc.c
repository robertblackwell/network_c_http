#include <src/common/alloc.h>
#include <assert.h>
#include <stdlib.h>
#include <rbl/macros.h>
#include <common/arena/arena.h>
#include <common/alloc_malloc.h>
Allocator* default_allocator_create()
{
#define ALLOCATOR_DEFAULT_ARENA
#ifdef ALLOCATOR_DEFAULT_ARENA
    Allocator* allocator = arena_allocator_create(1024*8);
#else
    Allocator* allocator = malloc_allocator_create();
#endif
    return allocator;
}


void* allocator_alloc(Allocator* allocator, size_t size)
{
    return allocator->allocate(allocator, size);
}
void* allocator_realloc(Allocator* allocator, void* old_ptr, size_t size)
{
    return allocator->reallocate(allocator, old_ptr, size);
}
void allocator_dealloc(Allocator* allocator, void* ptr)
{
    if(allocator->deallocate) {
        allocator->deallocate(allocator, ptr);
    }
}
void allocator_reset(Allocator* allocator)
{
    if(allocator->reset) {
        allocator->reset(allocator);
    }
}
void allocator_destroy(Allocator* allocator)
{
    RBL_ASSERT((allocator != NULL), "allocator_destroy - allocator is not null")
    if(allocator->destroy != NULL) {
        allocator->destroy(allocator);
    }
}
