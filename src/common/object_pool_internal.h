
#ifndef H_runloop_object_pool_allocator_internal_H
#define H_runloop_object_pool_allocator_internal_H
#include "object_pool.h"
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <rbl/check_tag.h>

/**
 * A freelist is a circular containing uint16_t values. The size of the buffer
 * is determined at creation time (runtime) and cannot be expanded once created.
 *
 * The entries are indexes into an array of memory blocks an acts as the free list
 * for a memory allocation scheme managed by the ObjectPool struct and related functions.
 */
struct FreeList_s {
    RBL_DECLARE_TAG;
    // how many are on the list
    uint16_t    count;
    // the max number that can be put in the list
    uint16_t    max_entries;
    // the number to use for modulo arithmetic
    uint16_t    modulo_max;
    uint16_t    rdix;
    uint16_t    wrix;
    RBL_DECLARE_END_TAG;
    uint16_t    buffer[];
};

/**
 * Create a new free list with object_count entries
 */
FreeListRef freelist_new(int object_count);
/**
 *  tests a free list to see if its full
 */
bool freelist_is_full(FreeListRef fl);
/**
 * tests a free list to see if it is empty
 */
bool freelist_is_empty(FreeListRef fl);
/**
 *  add an entry to the end of the list
 */
void freelist_add(FreeListRef fl, uint16_t element);
/**
 * get and remove an entry from the front of the list
 */
uint16_t freelist_get(FreeListRef fl);
/**
 * the number of entries on the list
 */
size_t freelist_size(FreeListRef fl);

//const uint64_t open_tag = 'ABCDEFGH';
//const uint64_t close_tag = 'JKLMNOPQ';

struct ObjectPool_s {
    FreeList*   free_list_ptr;
    uint16_t    obj_size;
    uint16_t    obj_count;
    uint16_t    blk_start_tag_offset; // offset from the start of a memory block to the beginning of the start tag
    uint16_t    blk_index_offset;     // offset from the start of a memory block to the block index value
    uint16_t    blk_end_tag_offset;
    uint16_t    blk_tag_length;
    uint16_t    stride;
    void*       memory_blocks;
};


uint16_t object_pool_stride(ObjectPoolRef ot);
uint16_t object_pool_obj_size(ObjectPoolRef ot);
uint16_t object_pool_obj_count(ObjectPoolRef ot);
/**
 * return the address (a pointer to) the index-th memory block
 * in the pool
 */
void* block_at(ObjectPool* pool, unsigned int index);
/**
 * @param void* blk_ptr a pointer to a memory block in an object pool
 * @return the anonymous address of the object memory inside the block
 */
void* blk_get_object_start(ObjectPool* pool, void* blk_ptr);
/**
 *
 * @param void* blk_ptr - a pointer to a memory block
 * @return the uint16_t index value held inside an allocated block
 */
uint16_t blk_get_index(ObjectPool* pool, void* blk_ptr);
/**
 * Set the index value for a memory block
 * @param blk_ptr a pointer to an object pool memory block
 * @param index the utin16_t index value to be set/givento the block
 */
void blk_set_index(ObjectPool* pool, void* blk_ptr, uint16_t index);
/**
 * Set  the starting tag value for a memory block
 * @param blk_ptr
 * @param tag_value
 */
void blk_set_tag(ObjectPool* pool, void* blk_ptr, char*  tag_value);
/**
 * Check that the memory block pointed at by blk_ptr has a beginning tag
 * with the expected value. assert if not
 * @param blk_ptr
 * @param expected_value
 */
void blk_check_tag(ObjectPool* pool, void* blk_ptr, char* expected_value);
/**
 * Set  the ending tag value for a memory block
 * @param blk_ptr
 * @param tag_value
 */
void blk_set_end_tag(ObjectPool* pool, void* blk_ptr, char*  tag_value);
/**
 * Check that the memory block pointed at by blk_ptr has an ending tag
 * with the expected value. assert if not
 * @param blk_ptr
 * @param expected_value
 */
void blk_check_end_tag(ObjectPool* pool, void* blk_ptr, char* expected_value);
/**
 * For testing purposes only - fill the entire object portion of a memory block
 * with a const character value
 * @param blk_ptr
 * @param ch
 * @param obj_size
 */
void blk_fill_object(ObjectPool* pool, void* blk_ptr, char ch, int obj_size);
/**
 * For testing purposes only
 * - set the start and end tags of a block
 * - fill the entire object portion of a memory block with a const character value
 *
 * The effect is to write data to every location inside a block - the hope is this will
 * cause a problem if we have calculated the location and extend of a block incorrectly.
 *
 * @param blk_ptr
 * @param ch
 * @param obj_size
 */
void mark_block(ObjectPool* ot, unsigned int index);
/**
 * Determine the address of the block that an object resides in.
 * Allso checks the start and end tags are correct and assert() if not
 * @param ot
 * @param obj_ptr
 * @return
 */
void* blkptr_from_objptr(ObjectPool* ot, void* obj_ptr);


#endif