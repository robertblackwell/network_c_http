//opaque type representing list
#include "alloc_object_pool.h"
#include "alloc_object_pool_internal.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <rbl/check_tag.h>
#include <src/common/utils.h>
#include <rbl/macros.h>
#include "rbl/macros.h"

#define OpBlockList_TAG "OPBLKL"

OpBlockList* op_block_list_new(size_t link_offset)
{
    OpBlockList* list = malloc(sizeof(OpBlockList));
    RBL_ASSERT((list != NULL), "malloc returned NULL");
    op_block_list_init(list, link_offset);
    return list;
}

void op_block_list_init(OpBlockList* list, size_t link_offset)
{
    ASSERT_NOT_NULL(list);
    OBJECT_POOL_SET_TAG(OpBlockList_TAG, list)
    OBJECT_POOL_SET_END_TAG(OpBlockList_TAG, list)
    list->count = 0;
    list->head = NULL;
    list->tail = NULL;
}

void op_block_list_free(OpBlockList* list)
{
    ASSERT_NOT_NULL(list);
    OBJECT_POOL_CHECK_TAG(OpBlockList_TAG, list)
    OBJECT_POOL_CHECK_END_TAG(OpBlockList_TAG, list)
    RBL_ASSERT((list->count == 0), "Free-ing non empty list");
    free(list);
}
size_t op_block_list_size(const OpBlockList* list)
{
    OBJECT_POOL_CHECK_TAG(OpBlockList_TAG, list)
    OBJECT_POOL_CHECK_END_TAG(OpBlockList_TAG, list)
    return list->count;
}
void op_block_list_display(OpBlockList* list)
{
    OBJECT_POOL_CHECK_TAG(OpBlockList_TAG, list)
    OBJECT_POOL_CHECK_END_TAG(OpBlockList_TAG, list)
    printf("MblockList[%p] count: %ld head %p tail %p\n", (void*)list, list->count, (void*)list->head, (void*)list->tail);
    MemoryBlock* iter = list->head;
    while(iter != NULL) {
        printf("Node[%p] forward:%p backwards:%p  item:%p\n", (void*)iter, (void*)iter->next_block, (void*)iter->prev_block, iter);
        MemoryBlock* next = iter->next_block;
        iter = next;
    }
}
void op_block_list_empty(const OpBlockList* list)
{
    MemoryBlock* b = list->head;
    while(b != NULL) {
        MemoryBlock* next = b->next_block;
        // free(b);
        b = next;
    }
}
void* op_block_list_find(const OpBlockList* list, const MemoryBlock* node)
{
    OBJECT_POOL_CHECK_TAG(OpBlockList_TAG, list)
    OBJECT_POOL_CHECK_END_TAG(OpBlockList_TAG, list)
    MemoryBlock* iter = list->head;
    while(iter != NULL) {
        if(node == iter) {
            return iter;
        }
        iter = iter->next_block;
    }
    return NULL;
}
MemoryBlock* op_block_list_remove_first(OpBlockList* list)
{
    OBJECT_POOL_CHECK_TAG(OpBlockList_TAG, list)
    OBJECT_POOL_CHECK_END_TAG(OpBlockList_TAG, list)
    if(list->head == NULL) {
        return NULL;
    }
    MemoryBlock* blk = list->head;
    if(list->head == list->tail) {
        list->head = NULL;
        list->tail = NULL;
    } else {
        list->head = list->head->next_block;
        list->head->prev_block = blk->prev_block;
    }
    return blk;
}
MemoryBlock* op_block_list_remove(OpBlockList* list, MemoryBlock* node)
{
    OBJECT_POOL_CHECK_TAG(OpBlockList_TAG, list)
    OBJECT_POOL_CHECK_END_TAG(OpBlockList_TAG, list)
    RBL_ASSERT((op_block_list_find(list, node) != NULL), "removing node from wrong list")
    MemoryBlock* tmp = op_block_list_find(list, node);
    if(tmp != node) {
        return NULL;
    }
    if(node == list->head && node == list->tail) {
        list->head = NULL;
        list->tail = NULL;
    } else if(list->head == node) {
        list->head = node->next_block;
        list->head->prev_block = node->prev_block;
    } else if(list->tail == node) {
        list->tail = node->prev_block;
        list->tail->next_block = node->next_block;
    } else {
        node->prev_block->next_block = node->next_block;
        node->next_block->prev_block = node->prev_block;
    }
    list->count--;
    node->next_block = NULL;
    node->prev_block = NULL;
    return node;
}
// add to the list in descending order of size
void op_block_list_add(OpBlockList* list, MemoryBlock* node)
{
    ASSERT_NOT_NULL(list);
    OBJECT_POOL_CHECK_TAG(OpBlockList_TAG, list)
    OBJECT_POOL_CHECK_END_TAG(OpBlockList_TAG, list)
    if(list->head == NULL) {
        list->head = node;
        list->tail = node;
        node->next_block = NULL;
        node->prev_block = NULL;
    } else {
        node->next_block = list->head;
        node->prev_block = NULL;
        list->head->prev_block = node;
        list->head = node;
    }
    list->count++;
}
