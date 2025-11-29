#ifndef c_http_threadlocal_freelist_h
#define c_http_threadlocal_freelist_h
#include "mblock.h"
struct MBlockList_s;
typedef struct MBlockList_s MBlockList, *MBlockListRef;

#define FreeList_TAG "BFRLIST"

typedef struct MBlockList_s MBlockList;
struct MBlockList_s
{
    RBL_DECLARE_TAG;
    MBlock* head;
    MBlock* tail;
    size_t count;
    RBL_DECLARE_END_TAG;
};

MBlockList* tl_freelist_new();
void tl_freelist_init(MBlockList* list);
void tl_freelist_free(MBlockList* list);
void tl_freelist_empty(const MBlockList* list);
void tl_freelist_display(MBlockList* list);
size_t tl_freelist_size(const MBlockList* list);

/**
 * Add a free block to the list maintaining decreasing free space size order
 */
void tl_freelist_add(MBlockList* list, MBlock* block);
/**
 * Find an MBlock on the free list that has enough freespace.
 * The function return value is a pointer to the bock.
 * The block is still on the freelist block.
 * The blocks free_space may be much larger than needed in which case you might like to split
 * the block. That depends on your allocation strategy.
 */
MBlock* tl_freelist_find_space(const MBlockList* list, size_t user_space_required);
/**
 * Search the free list for a MBlock that is a candidate for merging with the parameter block.
 * Returns a poiter to the candidate but do not remove it
 */
MBlock* tl_freelist_find_merge(MBlockList* list, MBlock* block);

/**
 * Remove the block parameter from the list
 */
void tl_freelist_remove(MBlockList* list, MBlock* block);

/**
 *  Find the block denoted by needle parameter in the list. Return
 *  needle if found NULL if not
 */
void* tl_freelist_find(const MBlockList* list, const MBlock* needle);
#endif