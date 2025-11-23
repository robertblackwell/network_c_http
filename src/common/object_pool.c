#include "object_pool.h"
#include "object_pool_internal.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#define FREELIST_Tag "FLTAG"


FreeList* freelist_new(int obj_count)
{
    size_t x = sizeof(FreeList) + sizeof(uint16_t)*obj_count;
    FreeList* fl = malloc(sizeof(FreeList) + sizeof(uint16_t)*obj_count);
    RBL_SET_TAG(FREELIST_Tag, fl)
    RBL_SET_END_TAG(FREELIST_Tag, fl)
    fl->count = 0;
    fl->rdix = 0;
    fl->wrix = 0;
    fl->max_entries = obj_count; //EVT_MAX;
    fl->modulo_max = obj_count+1; //EVT_MAX+1;
    
    for(int i = 0; i < fl->max_entries; i++) {
        freelist_add(fl, i);
    }
    return fl;
}
void freelist_free(FreeListRef fl)
{
    free(fl);
}
bool freelist_is_full(FreeListRef fl)
{
    return fl->count == (fl->max_entries);
}
bool freelist_is_empty(FreeListRef fl)
{
    return (fl->count == 0);
}
void freelist_add(FreeListRef fl, uint16_t element)
{
    assert(!freelist_is_full(fl));
    fl->count++;
    fl->buffer[fl->wrix] = element;
    fl->wrix = (fl->wrix + 1) % fl->modulo_max; 
}
uint16_t freelist_get(FreeListRef fl)
{
    assert(!freelist_is_empty(fl));
    uint16_t v = fl->buffer[fl->rdix];
    fl->count--;
    fl->rdix = (fl->rdix + 1) % fl->modulo_max;
    if(fl->count == 0) {
        fl->rdix = 0;
        fl->wrix = 0;
    }
    return v;
}
size_t freelist_size(FreeListRef fl)
{
    return fl->count;
}
/**
 * The following struct ObjectPool_s holds a a slab of memory and values used to manage that memory
 * so that pieces of it can be allocated out to hold fixed sized objects.
 *
 * When an instance of this type is created to manage a pool of objects it is made big enough
 * so that following the `stride` property there is enough continguous memory to hold `obj_count`
 * blocks of memory where each block is big enough to hold an object of size `obj_size` plus some
 * block specific ontrol information.
 *
 * The FreeList* field is a circular buffer holding 16 bit integers that act as indexes
 * into a slab of memory that starts at the address of the `memory_blocks` property.
 *
 * The memory blocks are laid out contiguously and the `stride` property is the size of the memory address
 * difference between consecutive blocks. So that is `void* ptr` is the address of a block then `ptr+stride`
 * is the address of the next block.
 */
//struct ObjectPool_s {
//    FreeList*   free_list_ptr;
//    uint16_t    obj_size;
//    uint16_t    obj_count;
//    uint16_t    stride;
//    void*       memory_blocks; // pointer to array of pointers to memory blocks
//};

/**
 * A single memory block conceptually looks like the following struct.
 */
typedef struct MemoryBlock_s {
    char  tag[8];             // the address of the start of this field is (char*)&(p->tag)
    uint16_t   blk_index;     // the addr of the start of this field is (uint16_t)&(p->blk_index)
    void*      blk_start;     // the addr of the start of the obj held in this block is (void*)&(p->blk_start)
    //...... in here is the continuoous memory that will actually be provided
    // to the user code as a place to put an instance of an object
    char   end_tag[8];        // the addr of this field is ((char*)&(p->blk_start)
} MemoryBlock, *MemoryBlockPtr;

 /**
  * Note the comment just before the end_tag. Because of that fact we cannot accurately describe
  * a memory block with a struct. Instead we have to use numeric offsets.
  *
  * the tag and end_tag fields are debugging tricks for use during development to help catch "buffer overruns".
 *
 * We can create one of these blocks with malloc(8 + 8 + obj_size + 8) and we can allocate the memory for
 * N consecutive blocks with malloc(N * (8+8+obj_size+8)). Where does this come from ?
  *
  * char     tag[8] is 8 bytes in size
  * uint16_t blk_index; is 2 bytes in size but because the next field must be 8 bytes alighed
  *                      it effectively takes 8 bytes
  * void*    blk_start  is 8 bytes aligned and is obj_size which is also a multiple of 8
  * char     end_tag[8] is 8 bytes in size amd 8 bytes aligned.
 *
 * We call the quantity (8+8+obj_size+8) the stride - it is the size of the steps or strides between
 * consecutive blocks. Thus if p is an anonymous pointer to the start of one block then (p+stride)
 * is a pointer to the next block.
 *
 * If p is a pointer to the start of one of these blocks we can extract and set the fields as follows.
 *
 * Access to fields:
 *
 *  -   (char*)&(p->tag) or (char*)(p) is an expression for the address of the tag field. The strncpy() function can be used to get or set the
 *      8 bytes value of this field. the tag field has offset 0 frm the start of the struct
 *
 *  -   (uint16_t)&(p->blk_index) or (uint16_t)(p+8) is an expression for the address of the blk_index field. The value can be got or set
 *      by assignment
 *          -   uint16_t x = *(uint16_t)(p + 8) retrieves the value
 *          -   *(uint16_t)(p + 8)) = 123 will set the field
 *
 *  -   (void*)(p + 16) is an expression for the start of the object that occupies the block. It
 *      occupies memory addreses (p + 16) through ((p + 16) + stride - 1)
 *
 *  -   (char*)(p + 16 + obj_size ) is an expression for the start of the end_tag field. The functions
 *      strncpy(), memcpt() and memset() can be used to set and get this field.
 *
 */


//////////////////////////////////////////////////////////////////////////////////////////////////
//typedef struct MBlk {
//    char        tag[8];
//    uint16_t    blk_index;
//}MBlk;
void* block_at(ObjectPool* op, unsigned int index)
{
    unsigned long stride = 8+8+op->obj_size+8;
    void* p = &(op->memory_blocks);
    void* r = p + (index * stride);
    return r;
}
void* blk_get_object_start(ObjectPool* pool, void* blk_ptr)
{
    return blk_ptr+16;
}
void* blkptr_from_objptr(ObjectPool* op, void* obj_ptr)
{
    void* blk_ptr = obj_ptr - 16;
    blk_check_tag(op, blk_ptr, "ABCDEFGH");
    blk_check_end_tag(op, blk_ptr,  "JKLMNOPQ");
    uint16_t ix = blk_get_index(op, blk_ptr);
    void* blk_tmp = block_at(op, ix);
    if(blk_ptr != blk_tmp) {
        assert(0);
    }
    return blk_ptr;
}
uint16_t blk_get_index(ObjectPool* pool, void* blk_ptr)
{
    return *(uint16_t*)(blk_ptr + 8);
}
void blk_set_index(ObjectPool* pool, void* blk_ptr, uint16_t index)
{
    *(uint16_t*)(blk_ptr + pool->blk_index_offset) = index;
}
void blk_set_tag(ObjectPool* pool, void* blk_ptr, char*  tag_value)
{
    strncpy((blk_ptr + pool->blk_start_tag_offset), tag_value, 8);
}
void blk_check_tag(ObjectPool* pool, void* blk_ptr, char* expected_value)
{
    if(strncmp(blk_ptr, expected_value, 8) != 0) {
        assert(0);
    }
}
void blk_set_end_tag(ObjectPool* pool, void* blk_ptr, char*  tag_value)
{
    strncpy((blk_ptr + pool->blk_end_tag_offset), tag_value, 8);
}
void blk_check_end_tag(ObjectPool* pool, void* blk_ptr, char* expected_value)
{
    assert(strncmp((blk_ptr + pool->blk_end_tag_offset), expected_value, 8) == 0);
}
void blk_fill_object(ObjectPool* pool, void* blk_ptr, char ch, int obj_size)
{
    memset((blk_ptr + 16), ch, obj_size);
}
void mark_block(ObjectPool* op, unsigned int index)
{
    void* block_ptr = block_at(op, index);
    blk_set_tag(op, block_at(op, index), "ABCDEFGH");
    blk_set_index(op, block_at(op, index), (uint16_t)0x0123456);
    blk_set_end_tag(op, block_at(op, index),  "JKLMNOPQ");
    blk_fill_object(op, block_at(op, index), 'z', op->obj_size);
}
//static void set_object_index(ObjectPool* otptr, uint16_t index)
//{
////    uint16_t* p = ((otptr->objects[index]) + otptr->obj_size);
////    *p = index;
//}
//static uint16_t get_object_index(ObjectPool* otptr, void* obj_ptr)
//{
////    uint16_t* p = (obj_ptr + otptr->obj_size);
////    return *p;
//}
ObjectPool* object_pool_create(int obj_size, int obj_count) {
    typedef struct MBlk {
        char tag[8];
        uint16_t blk_index;
    } MBlk;
    int blk_size = 8+8+obj_size+8;
    ObjectPool *et = malloc(sizeof(ObjectPool) + obj_count * blk_size);
    et->free_list_ptr = freelist_new(obj_count);
    et->obj_count = obj_count;
    et->obj_size = obj_size;
    uint16_t vsize = sizeof(void*);
    int xx = obj_size / vsize;
    assert((obj_size / vsize) * vsize == obj_size); // check alignment
    MBlk *p = 0;
    et->blk_tag_length = 8;
    et->blk_start_tag_offset = (uint16_t) ((void *) &(p->tag) - (void *) p);
    assert(et->blk_start_tag_offset == 0);
    et->blk_index_offset = (uint16_t) ((void *) &(p->blk_index) - (void *) p);
    assert(et->blk_index_offset == et->blk_tag_length);
    et->stride = 8 + 8 + obj_size + 8;
    et->blk_end_tag_offset = et->stride - et->blk_tag_length;
    return et;
}
void object_pool_destroy(ObjectPoolRef pool)
{
    freelist_free(pool->free_list_ptr);
    free(pool);
}
void* object_pool_allocate(ObjectPoolRef op)
{
    uint16_t ix = freelist_get((op->free_list_ptr));
    void* blkptr = block_at(op, ix);
    blk_set_index(op, blkptr, ix);
    blk_set_tag(op, blkptr, "ABCDEFGH");
    blk_set_end_tag(op, blkptr, "JKLMNOPQ");
    void* objptr = blk_get_object_start(op, blkptr);
    return objptr;
}
void object_pool_deallocate(ObjectPoolRef op, void* p)
{
    void* blkp = blkptr_from_objptr(op, p);
    blk_check_tag(op, blkp, "ABCDEFGH");
    blk_check_end_tag(op, blkp, "JKLMNOPQ");
    uint16_t ix = blk_get_index(op, blkp);
    freelist_add((op->free_list_ptr), ix);
}
size_t object_pool_number_in_use(ObjectPoolRef et)
{
    FreeListRef fl = (et->free_list_ptr);
    size_t fl_unused = freelist_size(fl);
    size_t fl_used = (fl->max_entries) - fl_unused;
    return fl_used;
}
bool object_pool_has_outstanding_objects(ObjectPoolRef et)
{
    return ! freelist_is_full((et->free_list_ptr));
}
uint16_t object_pool_stride(ObjectPool* op)
{
    return op->stride;
}
uint16_t object_pool_obj_count(ObjectPool* op)
{
    return op->obj_count;
}
uint16_t object_pool_obj_size(ObjectPool* op)
{
    return op->obj_size;
}