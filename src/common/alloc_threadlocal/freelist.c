//opaque type representing list
#include "freelist.h"
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <rbl/check_tag.h>
#include <src/common/utils.h>
#include <rbl/macros.h>
#include "mblock.h"
#include "rbl/macros.h"

MBlockList* tl_freelist_new()
{
    MBlockList* list = malloc(sizeof(MBlockList));
    RBL_ASSERT((list != NULL), "malloc returned NULL");
    tl_freelist_init(list);
    return list;
}

void tl_freelist_init(MBlockList* list)
{
    ASSERT_NOT_NULL(list);
    RBL_SET_TAG(FreeList_TAG, list)
    RBL_SET_END_TAG(FreeList_TAG, list)
    list->count = 0;
    list->head = NULL;
    list->tail = NULL;
}

void tl_freelist_free(MBlockList* list)
{
    ASSERT_NOT_NULL(list);
    RBL_ASSERT((list->count == 0), "Free-ing non empty list");
    free(list);
}
size_t tl_freelist_size(const MBlockList* list)
{
    return list->count;
}
void tl_freelist_display(MBlockList* list)
{
    printf("MblockList[%p] count: %ld head %p tail %p\n", (void*)list, list->count, (void*)list->head, (void*)list->tail);
    MBlock* iter = list->head;
    while(iter != NULL) {
        printf("Node[%p] forward:%p backwards:%p  item:%p  %ld\n", (void*)iter, (void*)iter->forward, (void*)iter->backward, iter, iter->free_space_size);
        MBlock* next = iter->forward;
        iter = next;
    }
}
void tl_freelist_empty(const MBlockList* list)
{
    MBlock* b = list->head;
    while(b != NULL) {
        MBlock* next = b->forward;
        free(b);
        b = next;
    }
}
MBlock* tl_freelist_find_space(const MBlockList* list, size_t required_user_size)
{
    MBlock* iter = list->head;
    if(iter == NULL) return NULL;
    if(iter->free_space_size < required_user_size) return NULL;
    while(1) {
        if(iter->forward == NULL) return iter;
        if(iter->forward->free_space_size < required_user_size) return iter;
        iter = iter->forward;
    }
    RBL_ASSERT(0, "Sould not get here");
}
void* tl_freelist_find(const MBlockList* list, const MBlock* node)
{
    MBlock* iter = list->head;
    while(iter != NULL) {
        if(node == iter) {
            return iter;
        }
        iter = iter->forward;
    }
    return NULL;
}
void tl_freelist_remove(MBlockList* list, MBlock* node)
{
    RBL_ASSERT((tl_freelist_find(list, node) != NULL), "removing node from wrong list")
    if(node == list->head && node == list->tail) {
        list->head = NULL;
        list->tail = NULL;
    } else if(list->head == node) {
        list->head = node->forward;
        list->head->backward = node->backward;
    } else if(list->tail == node) {
        list->tail = node->backward;
        list->tail->forward = node->forward;
    } else {
        node->backward->forward = node->forward;
        node->forward->backward = node->backward;
    }
    list->count--;
    node->forward = NULL;
    node->backward = NULL;
}
// add to the list in descending order of size
void tl_freelist_add(MBlockList* list, MBlock* node)
{
    ASSERT_NOT_NULL(list);
    node->forward = NULL;
    node->backward = NULL;
    if(list->head == NULL) {
        list->head = node;
        list->tail = node;
        list->count++;
        return;
    }
    MBlock* iter = list->head;
    while(1) {
        if(node->free_space_size >= iter->free_space_size) {
            if(iter->backward == NULL) {
                list->head = node;
                node->forward = iter;
                node->backward = iter->backward;
                iter->backward = node;
                break;
            } else {
                node->forward = iter;
                node->backward = iter->backward;
                iter->backward->forward = node;
                iter->backward = node;
                break;
            }
        }
        if(iter->forward == NULL) {
            list->tail = node;
            node->forward = NULL;
            node->backward = iter;
            iter->forward = node;
            break;
        }
        iter = iter->forward;
    }
}
MBlock* tl_freelist_find_merge(MBlockList* list, MBlock* block)
{
    ASSERT_NOT_NULL(list);
    MBlock* iter = list->head;
    while(iter != NULL) {
        if(memblock_adjacent(iter, block)) {
            return iter;
        }
        iter = iter->forward;
    }
    return NULL;
}
#if 0
MBlock* tl_freelist_find_space(MBlockList* list, size_t user_space_required)
{
    ASSERT_NOT_NULL(list);
    MBlock* iter = list->head;
    while(iter != NULL) {
        if((iter->free_space_size >= user_space_required)
            && ((iter->forward !=NULL) &&(iter->forward->free_space_size < user_space_required))) {
            return iter;
        }
        iter = iter->forward;
    }
    return NULL;
}
void tl_freelist_remove(MblockList* lref, MBlock* node_ptr)
{
    ASSERT_NOT_NULL(lref);
    ASSERT_NOT_NULL(node_ptr);
    MBlock* itr = node_ptr;
    ASSERT_NOT_NULL(itr);
    if(lref->count == 0)
        return;
    else if(lref->count == 1 ) {
        assert(itr == lref->head);
        lref->count = 0;
        lref->head = NULL;
        lref->tail = NULL;
        return;
    }
    if(lref->head == itr) {
        (itr)->forward->backward = (itr)->backward;
        lref->head = (itr)->forward;
    } else if (lref->tail == itr) {
        itr->backward->forward = itr->forward;
        lref->tail = itr->backward;
    } else {
        itr->forward->backward = itr->backward;
        itr->backward->forward = itr->forward;
    }
    lref->count--;
}
#endif
