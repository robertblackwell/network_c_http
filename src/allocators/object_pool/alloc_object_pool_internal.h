
#ifndef H_runloop_object_pool_allocator_internal_H
#define H_runloop_object_pool_allocator_internal_H
#include "alloc_object_pool.h"
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <rbl/check_tag.h>
#define MemoryBlock_TAG "MEMBLK"
#define MemoryBlock_ETAG "MEMBLKND"
#define OjectPool_TAG "OBJPOOL"
#define OjectPool_ETAG "OBJPOOLE"
/**
 * A single memory block conceptually looks like the following struct.
 */
typedef struct MemoryBlock_s MemoryBlock, *MemoryBlockPtr;
typedef struct OpBlockList_s OpBlockList;

void op_block_list_init(OpBlockList* list, size_t link_offset);
void op_block_list_deinit(const OpBlockList* list);
void op_block_list_display(OpBlockList* list);
size_t op_block_list_size(const OpBlockList* list);
void op_block_list_add(OpBlockList* list, MemoryBlock* block);
MemoryBlock* op_block_list_remove(OpBlockList* list, MemoryBlock* block);
MemoryBlock* op_block_list_remove_first(OpBlockList* list);
void* op_block_list_find(const OpBlockList* list, const MemoryBlock* needle);

char* op_block_endtag_ptr(ObjectPool* op, MemoryBlock* blkp);
MemoryBlock* op_block_at(ObjectPool* op, size_t index);
void op_block_object_fill_check(ObjectPool* op, MemoryBlock* blkp, char fillch);

void op_block_init(ObjectPool* op, MemoryBlock* blk);
size_t op_block_size(size_t object_size);
uint16_t object_pool_stride(ObjectPoolRef ot);
//uint16_t object_pool_obj_size(ObjectPoolRef ot);
//uint16_t object_pool_obj_count(ObjectPoolRef ot);
///
/// Returns the number of bytes between the start of one memory block and the start
/// of the next block - for this allocator the result is constant
/// n
size_t op_block_stride(ObjectPool* op);
void* op_block_get_object_start(ObjectPool* pool, void* blk_ptr);
void op_block_set_tag(ObjectPool* pool, void* blk_ptr, char*  tag_value);
/**
 * Check that the memory block pointed at by blk_ptr has a beginning tag
 * with the expected value. assert if not
 * @param blk_ptr
 * @param expected_value
 */
void op_block_check_tag(ObjectPool* pool, void* blk_ptr, char* expected_value);
/**
 * Set  the ending tag value for a memory block
 * @param blk_ptr
 * @param tag_value
 */
void op_block_set_end_tag(ObjectPool* pool, void* blk_ptr, char*  tag_value);
/**
 * Check that the memory block pointed at by blk_ptr has an ending tag
 * with the expected value. assert if not
 * @param blk_ptr
 * @param expected_value
 */
void op_block_check_end_tag(ObjectPool* pool, void* blk_ptr, char* expected_value);
/**
 * For testing purposes only - fill the entire object portion of a memory block
 * with a const character value
 * @param blk_ptr
 * @param ch
 * @param obj_size
 */
void op_block_fill_object(ObjectPool* pool, void* blk_ptr, char ch, int obj_size);
/**
 * Determine the address of the block that an object resides in.
 * Allso checks the start and end tags are correct and assert() if not
 * @param ot
 * @param obj_ptr
 * @return
 */
void* op_block_blkptr_from_objptr(ObjectPool* ot, void* obj_ptr);

/**
 * In DEBUG mode - Checks the integrity of the tags at each end of a MemoryBlock
 */
void op_block_verify(ObjectPool* ot, MemoryBlock* blkp);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/// macro definitions - mostly related switching from DEBUG mode to NON_DEBUG mode
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define OBJECT_POOL_DEBUGX
#ifdef OBJECT_POOL_DEBUG
#define OBJECT_POOL_BLOCK_VERIFY(op, blkptr) op_block_verify(op, blkptr);
#define OBJECT_POOL_DECLARE_TAG         RBL_DECLARE_TAG
#define OBJECT_POOL_DECLARE_END_TAG     RBL_DECLARE_END_TAG

#define OBJECT_POOL_SET_TAG(tag_str, p) RBL_SET_TAG(tag_str, p)
#define OBJECT_POOL_SET_END_TAG(tag_str, p) RBL_SET_END_TAG(tag_str, p)

#define OBJECT_POOL_CHECK_TAG(tag_str, p) RBL_CHECK_TAG(tag_str, p)
#define OBJECT_POOL_CHECK_END_TAG(tag_str, p) RBL_CHECK_END_TAG(tag_str, p)

#define OBJECT_POOL_SET_TAG_PTR(tag_str, p) RBL_SET_TAG_PTR(tag_str, p)
#define OBJECT_POOL_CHECK_TAG_PTR(tag_str, p) RBL_CHECK_TAG_PTR(tag_str, p)

#else
#define OBJECT_POOL_BLOCK_VERIFY(op, blkptr)
#define OBJECT_POOL_DECLARE_TAG
#define OBJECT_POOL_DECLARE_END_TAG
#define OBJECT_POOL_SET_TAG(tag_str, p)
#define OBJECT_POOL_SET_END_TAG(tag_str, p)
#define OBJECT_POOL_CHECK_TAG(tag_str, p)
#define OBJECT_POOL_CHECK_END_TAG(tag_str, p)
#define OBJECT_POOL_SET_TAG_PTR(tag_str, p)
#define OBJECT_POOL_CHECK_TAG_PTR(tag_str, p)

#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////
/// struct definitions
////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct OpBlockList_s
{
    OBJECT_POOL_DECLARE_TAG;
    MemoryBlock* head;
    MemoryBlock* tail;
    size_t count;
    size_t link_offset;
    OBJECT_POOL_DECLARE_END_TAG
};

struct MemoryBlock_s {
    OBJECT_POOL_DECLARE_TAG
    MemoryBlock*    next_block;   // allows the block to be on the free list or the allocated list
    MemoryBlock*    prev_block;
    char*           file_name;
    size_t          line_number;
    size_t          blk_mem_size;
    uint8_t         mem[];       // the addr of the start of the obj held in this block is (void*)&(p->blk_start)
    //...... in here is the contiguous memory that will actually be provided
    // to the user code as a place to put an instance of an object
    //the is an end tag but it must be added and checked dynamically
};

struct ObjectPool_s {
    OBJECT_POOL_DECLARE_TAG
    OpBlockList     free_list;
    OpBlockList     allocated_list;
    uint16_t        obj_size;
    uint16_t        obj_count;
    void*           memory_blocks; // pointer to a slab of memory containing all the memory blocks to be allocated
    OBJECT_POOL_DECLARE_END_TAG
};

#endif