#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <rbl/macros.h>
#include "tl_allocator.h"
#include "mblock.h"
#include "intrusive_list.h"

static __thread Tlocal_Allocator* thread_local_allocator = NULL;

// struct Tlocal_Allocator_s
// {
//     size_t default_block_size;
//     MBlockList* free_list;
//     MBlockList* allocated_list;
//     MBlockList  free_list_mem;
//     MBlockList  allocated_list_mem;
// };
void tl_malloc_block_add(Tlocal_Allocator*tla, void* block)
{
    intptr_t* p = tla->malloc_blocks;
    p += tla->malloc_block_count;
    *p = (intptr_t)block;
    tla->malloc_block_count++;

}
void* tl_new_block(Tlocal_Allocator* tla, size_t block_size)
{
    void* bp = malloc(block_size);
    intptr_t* p = tla->malloc_blocks;
    p += tla->malloc_block_count;
    *p = (intptr_t)bp;
    tla->malloc_block_count++;
    return bp;
}
void tl_allocator_init(Tlocal_Allocator* tla)
{
    tla->free_list = &(tla->free_list_mem);
    tl_intrusive_list_init(tla->free_list);
    tla->allocated_list = &(tla->allocated_list_mem);
    tl_intrusive_list_init(tla->allocated_list);
    tla->default_block_size = 20*1024;
    tla->malloc_block_count = 0;
    tla->malloc_blocks = malloc(1000*sizeof(void*));
    thread_local_allocator = tla;
    MBlock* blk = memblock_new(tla->default_block_size);
    tl_malloc_block_add(tla, blk);
    tl_intrusive_list_add(tla->free_list, blk);
}
Tlocal_Allocator* tl_allocator_create()
{
    if(thread_local_allocator == NULL) {
        Tlocal_Allocator* tla = malloc(sizeof(Tlocal_Allocator));
        RBL_ASSERT((tla != NULL), "malloc returned NULL");
        tl_allocator_init(tla);
    }
    return thread_local_allocator;
}
void* tl_allocator_alloc(Tlocal_Allocator* tl, size_t size)
{
    MBlock* block = tl_intrusive_list_find_space(tl->free_list, size);
    if(block == NULL) {
        // printf("tl_alllocator_alloc NULL from free list find space\n");
        size_t blk_size = (memblock_size(size) > tl->default_block_size) ? memblock_size(size) : tl->default_block_size;
        block = memblock_new(blk_size);
        tl_malloc_block_add(tl, block);
        RBL_ASSERT((block->forward == NULL), "new block invariant failed");
        RBL_ASSERT((block->backward == NULL), "new block invariant failed");
        MBlock* nxt = memblock_endtag(block);
    } else {
        memblock_check_tags(block);
        tl_intrusive_list_remove(tl->free_list, block);
        RBL_ASSERT((block->forward == NULL), "new block invariant failed");
        RBL_ASSERT((block->backward == NULL), "new block invariant failed");
    }
    if(memblock_should_split(block, size)) {
        MBlock* remainder = memblock_split(block, size);
        tl_intrusive_list_add(tl->free_list, remainder);
    }
    // memblock_mark_allocated(block);
    tl_intrusive_list_add(tl->allocated_list, block);
    memblock_check_tags(block);

    void* p = memblock_user_ptr(block);
    RBL_ASSERT((p != NULL), "allocator trying to return NULL");
    return p;
}
void  tl_allocator_free(Tlocal_Allocator* tl, void* ptr)
{
    MBlock* block = memblock_from_userptr(ptr);
    memblock_check_tags(block);
    // memblock_check_allocated(block);
    tl_intrusive_list_remove(tl->allocated_list, block);
    RBL_ASSERT((block->forward == NULL), "remove block from list invariant failed");
    RBL_ASSERT((block->backward == NULL), "remove block from list invariant failed");
    while(1) {
        MBlock* merge_block = tl_intrusive_list_find_merge(tl->free_list, block);
        if(merge_block == NULL) {
            break;
        }
        tl_intrusive_list_remove(tl->free_list, merge_block);
        block = memblock_merge(block, merge_block);
    }
    // memblock_mark_free(block);
    tl_intrusive_list_add(tl->free_list, block);
}
void* tl_allocator_realloc(Tlocal_Allocator* tl, void* ptr, size_t size)
{
    void* newuser_ptr = tl_allocator_alloc(tl, size);
    MBlock* newblock = memblock_from_userptr(newuser_ptr);
    MBlock* old_block = memblock_from_userptr(ptr);
    memcpy(newuser_ptr, ptr, old_block->free_space_size);
    tl_allocator_free(tl, ptr);
    return newuser_ptr;
}
void tl_allocator_reset(Tlocal_Allocator* tl)
{
    // tl_intrusive_list_empty(tl->allocated_list);
    tl_intrusive_list_empty(tl->free_list);
    free(tl->malloc_blocks);
    thread_local_allocator = NULL;
    tl_allocator_init(tl);
}

