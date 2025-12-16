#include "arena.h"
#include "arena_internal.h"
#include <allocators/alloc.h>

void* api_arena_allocate(Allocator* allocator, size_t size)
{
    return arena_alloc((Arena*)allocator, size, NULL, 0);
}
void* api_arena_reallocate(Allocator* allocator, void* ptr, size_t size)
{
    return arena_realloc((Arena*)allocator , ptr, size, NULL, 0);
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

