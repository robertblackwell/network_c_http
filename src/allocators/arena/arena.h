#ifndef H_chttp_common_arena_H
#define H_chttp_common_arena_H
#include <inttypes.h>
#include <stddef.h>
#include <allocators/alloc.h>
typedef struct Arena_s Arena, *ArenaPtr;

#define ARENA_DEFAULT_CAPACITY 1024*4

Allocator* arena_allocator_create(size_t capacity);
ArenaPtr arena_create(size_t capacity);

#define ARENA_ALLOC(arena, alloc_size) arena_alloc(arena, alloc_size, __FILE__, __LINE__)
void* arena_alloc(ArenaPtr arena, size_t alloc_size, char* file, int linenumber);
#define ARENA_REALLOC(arena, ptr, alloc_size) arena_realloc(arena, ptr, alloc_size, __FILE__, __LINE__)
void* arena_realloc(ArenaPtr arena, void* ptr, size_t size, char* file, int linenumber);
void api_arena_deallocate(Allocator* allocator, void* ptr);
void arena_reset(ArenaPtr arena);
void arena_destroy(ArenaPtr arena);


#endif