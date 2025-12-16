//opaque type representing list
#include "mblock.h"
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <rbl/check_tag.h>
#include <rbl/macros.h>
#include <src/common/utils.h>
void* memblock_user_ptr(MBlock* memblk)
{
    RBL_CHECK_TAG(Block_Free_TAG, memblk)
    RBL_CHECK_TAG_PTR(Block_ENDTAG, (char*)memblock_endtag(memblk))
    void* p = &(memblk->mem[0]);
    return p;
}
size_t memblock_size(size_t free_space_size)
{
    size_t tag_length = ((RBL_TAG_LENGTH % 16) == 0) ? RBL_TAG_LENGTH: ((RBL_TAG_LENGTH / 16)+1) * 16;
    return sizeof(MBlock) + free_space_size + tag_length;
}
void memblock_init(MBlock* mblock, size_t free_space_size)
{
    RBL_SET_TAG(Block_Free_TAG, mblock)
    RBL_SET_TAG(Block_ENDTAG, (MBlock*)&(mblock->mem[free_space_size]))
    mblock->free_space_size = free_space_size;
    mblock->backward = NULL;
    mblock->forward = NULL;
}
void* memblock_endtag(MBlock* mblock)
{
    void* p = (MBlock*)&(mblock->mem[mblock->free_space_size]);
    return p;
}
void memblock_check_tags(MBlock* memblk)
{
    RBL_CHECK_TAG(Block_Free_TAG, memblk)
    RBL_CHECK_TAG_PTR(Block_ENDTAG, (char*)memblock_endtag(memblk))
}
void memblock_check_allocated(MBlock* memblk)
{
    RBL_CHECK_TAG(Block_Free_TAG, memblk)
    RBL_CHECK_TAG_PTR(Block_ENDTAG, (char*)memblock_endtag(memblk))
}
MBlock* memblock_from_userptr(void* userptr)
{
    size_t offset = offsetof(MBlock, mem);
    MBlock* memblk_ptr = (MBlock*)((char*)userptr - offset);
    RBL_CHECK_TAG(Block_Free_TAG, memblk_ptr)
    RBL_CHECK_TAG_PTR(Block_ENDTAG, (char*)memblock_endtag(memblk_ptr))
    return memblk_ptr;
}
MBlock* memblock_after(MBlock* memblk)
{
    // memblock_check_free_tags(memblk);
    size_t tag_length = ((RBL_TAG_LENGTH % 16) == 0) ? RBL_TAG_LENGTH: ((RBL_TAG_LENGTH / 16)+1) * 16;
    size_t offset = offsetof(MBlock, backward);
    void* p1 = &(memblk->mem[0]);
    void* p2 = &(memblk->mem[memblk->free_space_size]);
    MBlock* p = (MBlock*)(&(memblk->mem[memblk->free_space_size]) + tag_length);
    return p;
}
bool memblock_adjacent(MBlock* a, MBlock* b)
{
    RBL_CHECK_TAG(Block_Free_TAG, a)
    RBL_CHECK_TAG_PTR(Block_ENDTAG, (char*)memblock_endtag(a))
    RBL_CHECK_TAG(Block_Free_TAG, b)
    RBL_CHECK_TAG_PTR(Block_ENDTAG, (char*)memblock_endtag(b))
    if(b > a) {
        return b == memblock_after(a);
    }
    return a == memblock_after(b);
}
MBlock* memblock_new(size_t user_size)
{
    size_t alignment = _Alignof(max_align_t);
    user_size = (user_size % 16 == 0) ? user_size : ((user_size / 16)+1) * 16;
    size_t allosize = memblock_size(user_size);
    MBlock* mb = (MBlock*)malloc(allosize);
    memblock_init(mb, user_size);
    return mb;
}
size_t memblock_split_needed_freespace(size_t user_space_size)
{
    // see comments in thext function
    //
    size_t f1 = (user_space_size % 16 ==0) ? user_space_size : ((user_space_size / 16)+1) * 16;
    size_t h = offsetof(MBlock, mem);
    size_t t = ((RBL_TAG_LENGTH % 16) == 0) ? RBL_TAG_LENGTH: ((RBL_TAG_LENGTH / 16)+1) * 16;
    return h + f1 + t;
}
MBlock* memblock_split(MBlock* original, size_t user_size)
{
    RBL_CHECK_TAG(Block_Free_TAG, original)
    RBL_CHECK_TAG_PTR(Block_ENDTAG, (char*)memblock_endtag(original))
    // The original block consists of a Header + FreeSpace + Trailer denoted by H + F0 + T
    // Into this space must fit (H1 + FS1 + T1)+(H2 + F2 + T2)
    // H + F0 + T = H + F1 + T + H + F2 + T, simplifying gives
    // F0 = F1 + (T+H) + F2
    // F2 = F0 - F1 - T - H
    //
    size_t f1 = (user_size % 16 ==0) ? user_size : ((user_size / 16)+1) * 16;
    size_t f0 = original->free_space_size;
    size_t h = offsetof(MBlock, mem);
    size_t t = ((RBL_TAG_LENGTH % 16) == 0) ? RBL_TAG_LENGTH: ((RBL_TAG_LENGTH / 16)+1) * 16;
    size_t f2 = f0 - f1 - t - h;
    RBL_ASSERT(((h + f0 + t) == (h + f1 + t + h + f2 + t)), "block split invariant failed");
    original->free_space_size = f1;
    original->forward = NULL;
    original->backward = NULL;
    RBL_SET_TAG(Block_ENDTAG, (MBlock*)memblock_endtag(original))
    MBlock* mb = (MBlock*)memblock_after(original);
    memblock_init(mb, f2);
    return mb;
}
MBlock* memblock_merge(MBlock* a, MBlock* b)
{
    RBL_CHECK_TAG(Block_Free_TAG, a)
    RBL_CHECK_TAG_PTR(Block_ENDTAG, (char*)memblock_endtag(a))
    RBL_CHECK_TAG(Block_Free_TAG, b)
    RBL_CHECK_TAG_PTR(Block_ENDTAG, (char*)memblock_endtag(b))
// this is the reverse of split.
    // start with H1 + F1 + T1 and H2 + F2 + T2 and finish with H1 + (F1+H2+F2+T2) + T1
    // so the free space of the result is F1 + H2 + F2 + T2
    RBL_ASSERT((a != b), "a and b must not be the same");
    // printf("memblock_merge a:%p a->after %p b: %p  b->after%p\n", a, memblock_after(a), b, memblock_after(b));
    if(!memblock_adjacent(a, b)) {return NULL;}
    MBlock* result = (a < b) ? a : b;
    MBlock* other = (a < b) ? b : a;
    size_t h = offsetof(MBlock, mem);
    size_t t = ((RBL_TAG_LENGTH % 16) == 0) ? RBL_TAG_LENGTH: ((RBL_TAG_LENGTH / 16)+1) * 16;
    size_t f1 = result->free_space_size;
    result->free_space_size = f1 + (h + other->free_space_size + t);
    // printf("result->free_space_size: %zu\n", result->free_space_size);
    return result;
}
bool memblock_should_split(MBlock* block, size_t user_size)
{
    RBL_CHECK_TAG(Block_Free_TAG, block)
    RBL_CHECK_TAG_PTR(Block_ENDTAG, (char*)memblock_endtag(block))
    bool x = (block->free_space_size > 2 * user_size) &&(user_size > 64);
    return x;
}
void memblock_mark_allocated(MBlock* block)
{
    RBL_CHECK_TAG(Block_Free_TAG, block)
    RBL_CHECK_TAG_PTR(Block_ENDTAG, (char*)memblock_endtag(block))
}
void memblock_mark_free(MBlock* block)
{
    RBL_CHECK_TAG(Block_Free_TAG, block)
    RBL_CHECK_TAG_PTR(Block_ENDTAG, (char*)memblock_endtag(block))
}
