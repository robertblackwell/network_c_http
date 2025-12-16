#ifndef c_http_threadlocal_mblock_h
#define c_http_threadlocal_mblock_h
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <rbl/check_tag.h>
#include <src/common/utils.h>
//Internal - type used to build list

#define Block_Allocated_TAG    "MEMBLK___ALLOC"
#define Block_Free_TAG         "MEMBLK____FREE"
#define Block_ENDTAG           "MEMBLK_____END"
// #define MBLK_FREE_TAG 'MEMBLKF'
// #define MBLK_ALLOCATE_TAG 'MEMBLKA'
// #define MBLK_DECLARE_TAG union {uint64_t uinttag; char chtag[8];} tag;
// #define MBLK_SET_TAG(tagvalue, p) do {p->tag.uinttag = tagvalue;} while(0);
// #define MBLK_CHECK_TAG(tagvalue, p) do {assert(p->tag.uinttag = tagvalue); } while(0);
typedef struct MBlock_s MBlock;
struct MBlock_s {
    RBL_DECLARE_TAG;
    MBlock* forward;
    MBlock* backward;
    size_t free_space_size;
    uint8_t mem[];
    // RBL_DECLARE_TAG after the vaiable sized mem[] field
};
void*   memblock_user_ptr(MBlock* memblk);
size_t  memblock_size(size_t free_space_size);
void    memblock_init(MBlock* mblock, size_t free_space_size);
void*   memblock_endtag(MBlock* mblock);
void    memblock_check_tags(MBlock* memblk);
MBlock* memblock_from_userptr(void* userptr);
MBlock* memblock_after(MBlock* memblk);
bool    memblock_adjacent(MBlock* a, MBlock* b);
MBlock* memblock_merge(MBlock* a, MBlock* b);
// void    memblock_mark_allocated(MBlock* block);
// void    memblock_check_allocated(MBlock* memblk);
// void    memblock_mark_free(MBlock* block);
// void    memblock_check_free(MBlock* memblk);

MBlock* memblock_new(size_t user_size);

/**
 * A convenience function when looking for a block to split to satisfy a request for memory.
 * This computes the amount of free space a block needs in order that it can be split
 * to satisy an allocation.
 */
size_t memblock_split_needed_freespace(size_t user_space_size);
/**
 * Split the original block to have just enough room for a user free_space
 * given by user_size.
 *
 * After the call the original has been resized to provide user_space_size
 *
 * If there is a left over block a ptr to it is the return value. You probably need to add it to
 * a free list
 */
MBlock* memblock_split(MBlock* original, size_t user_size);
bool memblock_should_split(MBlock* block, size_t required_user_size);

#endif