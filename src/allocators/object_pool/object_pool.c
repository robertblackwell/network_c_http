#include "alloc_object_pool.h"
#include "alloc_object_pool_internal.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>


//////////////////////////////////////////////////////////////////////////////////////////////////

ObjectPool* v2_object_pool_create(int obj_size, int obj_count) {
    typedef struct MBlk {
        char tag[8];
        uint16_t blk_index;
    } MBlk;
    ObjectPool *et = malloc(sizeof(ObjectPool));
    assert(et != NULL);
    op_block_list_init((OpBlockList*)&(et->allocated_list), 0);
    op_block_list_init((OpBlockList*)&(et->free_list), 0);
    et->obj_count = obj_count;
    size_t n = sizeof(void*);
    et->obj_size = (obj_size % n == 0)? obj_size: ((obj_size/n)+1)*n;
    size_t block_size = op_block_size(et->obj_size);
    size_t mem_size = block_size * et->obj_count;
    void* mem = malloc(mem_size);
    assert(mem != NULL);
    et->memory_blocks = mem;
    OBJECT_POOL_SET_TAG(OjectPool_TAG, et)
    OBJECT_POOL_SET_END_TAG(OjectPool_ETAG, et)
    size_t memcount = 0;
    uint8_t* p = (uint8_t*)(et->memory_blocks);
    for(int i = 0; i < et->obj_count; i++) {
        MemoryBlock* blkp = (MemoryBlock*)p;
        op_block_init(et, blkp);
        op_block_list_add((OpBlockList*)(&(et->free_list)), blkp);
        p += op_block_size(et->obj_size);
    }
    return et;
}
void v2_object_pool_destroy(ObjectPoolRef pool)
{
    OBJECT_POOL_CHECK_TAG(OjectPool_TAG, pool)
    OBJECT_POOL_CHECK_END_TAG(OjectPool_ETAG, pool)
    free(pool);
}
void* v2_object_pool_allocate(ObjectPoolRef op, char* file, size_t line_number)
{
    OBJECT_POOL_CHECK_TAG(OjectPool_TAG, op)
    OBJECT_POOL_CHECK_END_TAG(OjectPool_ETAG, op)
    if(op->free_list.head == NULL) {
        return NULL;
    }
    MemoryBlock* blkptr = op_block_list_remove_first((OpBlockList*)&(op->free_list));
    blkptr->file_name = file;
    blkptr->line_number = line_number;
    op_block_list_add((OpBlockList*)&(op->allocated_list), blkptr);
    OBJECT_POOL_BLOCK_VERIFY(op, blkptr);
    void* objptr = op_block_get_object_start(op, blkptr);
    return objptr;
}
void v2_object_pool_deallocate(ObjectPoolRef op, void* p, char* file, size_t line_number)
{
    OBJECT_POOL_CHECK_TAG(OjectPool_TAG, op)
    OBJECT_POOL_CHECK_END_TAG(OjectPool_ETAG, op)
    MemoryBlock* blkp = op_block_blkptr_from_objptr(op, p);
    OBJECT_POOL_BLOCK_VERIFY(op, blkp);
    MemoryBlock* tmp = op_block_list_remove((OpBlockList*)&(op->allocated_list), blkp);
    assert(tmp == blkp);
    blkp->file_name = file;
    blkp->line_number = line_number;
    op_block_list_add((OpBlockList*)&(op->free_list), blkp);
}
size_t v2_object_pool_number_in_use(ObjectPoolRef op)
{
    OBJECT_POOL_CHECK_TAG(OjectPool_TAG, op)
    OBJECT_POOL_CHECK_END_TAG(OjectPool_ETAG, op)
    return op->allocated_list.count;
}
bool v2_object_pool_has_outstanding_objects(ObjectPoolRef op)
{
    OBJECT_POOL_CHECK_TAG(OjectPool_TAG, op)
    OBJECT_POOL_CHECK_END_TAG(OjectPool_ETAG, op)
    return (op_block_list_size((OpBlockList*)&(op->allocated_list)) != 0);
}
uint16_t v2_object_pool_obj_count(ObjectPool* op)
{
    OBJECT_POOL_CHECK_TAG(OjectPool_TAG, op)
    OBJECT_POOL_CHECK_END_TAG(OjectPool_ETAG, op)
    return op->obj_count;
}
uint16_t v2_object_pool_obj_size(ObjectPool* op)
{
    OBJECT_POOL_CHECK_TAG(OjectPool_TAG, op)
    OBJECT_POOL_CHECK_END_TAG(OjectPool_ETAG, op)
    return op->obj_size;
}