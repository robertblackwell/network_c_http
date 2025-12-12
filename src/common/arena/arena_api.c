#include "arena.h"
#include "arena_internal.h"
#include <common/alloc.h>

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
