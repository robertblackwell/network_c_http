#ifndef H_c_http_common_tl_allocator_H
#define H_c_http_common_tl_allocator_H
#include <stddef.h>
#include "mblock.h"
#include "freelist.h"

typedef struct Tlocal_Allocator_s Tlocal_Allocator;
struct Tlocal_Allocator_s
{
    size_t default_block_size;
    MBlockList* free_list;
    MBlockList* allocated_list;
    MBlockList  free_list_mem;
    MBlockList  allocated_list_mem;
    size_t      malloc_block_count;
    intptr_t*   malloc_blocks;
};

Tlocal_Allocator* tl_allocator_create();
void* tl_allocator_alloc(Tlocal_Allocator* tl, size_t size);
void  tl_allocator_free(Tlocal_Allocator* tl, void* ptr);
void* tl_allocator_realloc(Tlocal_Allocator* tl, void* ptr, size_t size);
void tl_allocator_reset(Tlocal_Allocator* tl);
#endif