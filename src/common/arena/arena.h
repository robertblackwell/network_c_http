#ifndef H_chttp_common_arena_H
#define H_chttp_common_arena_H
#include <inttypes.h>
#include <stddef.h>
#include <common/alloc.h>
typedef struct Arena_s Arena, *ArenaPtr;

#define ARENA_DEFAULT_CAPACITY 1024*4

Allocator* arena_allocator_create(size_t capacity);
ArenaPtr arena_create(size_t capacity);
void arena_destroy(ArenaPtr arena);

void* arena_alloc(ArenaPtr arena, size_t alloc_size);
void* arena_realloc(ArenaPtr arena, void* ptr, size_t size);
void api_arena_deallocate(Allocator* allocator, void* ptr);
void arena_reset(ArenaPtr arena);


#endif