#include "alloc_object_pool.h"
#include "alloc_object_pool_internal.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>


//////////////////////////////////////////////////////////////////////////////////////////////////
void op_block_init(ObjectPool* op,  MemoryBlock* blkp)
{
    OBJECT_POOL_CHECK_TAG(OjectPool_TAG, op)
    OBJECT_POOL_CHECK_END_TAG(OjectPool_ETAG, op)
    OBJECT_POOL_SET_TAG(MemoryBlock_TAG, blkp)
    OBJECT_POOL_SET_TAG_PTR(MemoryBlock_ETAG, op_block_endtag_ptr(op, blkp))
    blkp->blk_mem_size = op->obj_size;
    blkp->next_block = NULL;
    blkp->prev_block = NULL;
}
MemoryBlock* op_block_at(ObjectPool* op, size_t index)
{
    OBJECT_POOL_CHECK_TAG(OjectPool_TAG, op)
    OBJECT_POOL_CHECK_END_TAG(OjectPool_ETAG, op)
    uint8_t* p = (uint8_t*)(op->memory_blocks);
    p += index * op_block_size(op->obj_size);
    MemoryBlock* blkp = (MemoryBlock*)p;
    return blkp;
}
char* op_block_endtag_ptr(ObjectPool* op, MemoryBlock* blkp)
{
    char* tag_ptr = (char*)(&(blkp->mem)) +  op->obj_size;
    return tag_ptr;
}
size_t op_block_size(size_t object_size)
{
    size_t n = sizeof(MemoryBlock) + object_size + rbl_tag_size();
    return n;
}
void* block_at(ObjectPool* op, unsigned int index)
{
    OBJECT_POOL_CHECK_TAG(OjectPool_TAG, op)
    OBJECT_POOL_CHECK_END_TAG(OjectPool_ETAG, op)
    unsigned long stride = 8+8+op->obj_size+8;
    void* p = &(op->memory_blocks);
    void* r = p + (index * stride);
    return r;
}
void* op_block_get_object_start(ObjectPool* pool, void* blk_ptr)
{
    OBJECT_POOL_CHECK_TAG(OjectPool_TAG, pool)
    OBJECT_POOL_CHECK_END_TAG(OjectPool_ETAG, pool)
    MemoryBlock* p = (MemoryBlock*)blk_ptr;
    OBJECT_POOL_BLOCK_VERIFY(pool, blk_ptr);
#if defined(OBJECT_POOL_DEBUG)
    size_t of0 = offsetof(MemoryBlock, tag);
#endif
    size_t of1 = offsetof(MemoryBlock, next_block);
    size_t of2 = offsetof(MemoryBlock, prev_block);
    size_t of3 = offsetof(MemoryBlock, blk_mem_size);
    size_t of4 = offsetof(MemoryBlock, mem);
    char* x1 = blk_ptr;
    char* x2 = x1 + of4;
    char* x3 = (char*)&(p->mem);
    char* x4 = x3 + pool->obj_size;
    return &(p->mem);
}
void* op_block_blkptr_from_objptr(ObjectPool* op, void* obj_ptr)
{
    OBJECT_POOL_CHECK_TAG(OjectPool_TAG, op)
    OBJECT_POOL_CHECK_END_TAG(OjectPool_ETAG, op)
    void* blk_ptr = obj_ptr - offsetof(MemoryBlock, mem);
    OBJECT_POOL_BLOCK_VERIFY(op, blk_ptr);
    return blk_ptr;
}
void op_block_check_tag(ObjectPool* pool, void* blk_ptr, char* expected_value)
{
    if(strncmp(blk_ptr, expected_value, 8) != 0) {
        assert(0);
    }
}
void op_block_fill_object(ObjectPool* pool, void* blk_ptr, char ch, int obj_size)
{
    OBJECT_POOL_CHECK_TAG(OjectPool_TAG, pool)
    OBJECT_POOL_CHECK_END_TAG(OjectPool_ETAG, pool)
    OBJECT_POOL_BLOCK_VERIFY(pool, blk_ptr)
    char* p = op_block_get_object_start(pool, blk_ptr);
    memset(p, ch, obj_size);
}
void op_block_verify(ObjectPool* op, MemoryBlock* blkp)
{
    OBJECT_POOL_CHECK_TAG(OjectPool_TAG, op)
    OBJECT_POOL_CHECK_END_TAG(OjectPool_ETAG, op)
    OBJECT_POOL_CHECK_TAG(MemoryBlock_TAG, blkp)
    OBJECT_POOL_CHECK_TAG_PTR(MemoryBlock_ETAG, op_block_endtag_ptr(op, blkp))
}
void op_block_object_fill_check(ObjectPool* op, MemoryBlock* blkp, char fillch)
{
    OBJECT_POOL_CHECK_TAG(OjectPool_TAG, op)
    OBJECT_POOL_CHECK_END_TAG(OjectPool_ETAG, op)
    OBJECT_POOL_BLOCK_VERIFY(op, blkp)
    char* p = (char*)(&(blkp->mem));
    for(int i = 0; i < op->obj_size; i++) {
        char tmp = *(p + i);
        assert(fillch == tmp);
    }
}