//opaque type representing list
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "dlist.h"
#include <src/common/utils.h>
//Internal - type used to build list

struct DListNode_s {
    DListNode* forward;
    DListNode* backward;
    void* item;
};

struct DList_s {
    int count;
    DListNode* head;
    DListNode* tail;
    DListItemDeallocator dealloc;
};

DListNode* DListNode_new(void* content, DListNode* prev, DListNode* next)
{
    DListNode* lnref = calloc(1, sizeof(DListNode));
    lnref->item = content;
    lnref->forward = next;
    lnref->backward = prev;
}
void DListNode_dispose( DListRef lref, DListNode** lnref_ptr)
{
    ASSERT_NOT_NULL(lref);
    ASSERT_NOT_NULL(lnref_ptr);
    DListNode* lnref = *lnref_ptr;
    if(lref->dealloc != NULL) {
        if(lnref->item != NULL)
            lref->dealloc(&(lnref->item));
    }
    free((void*)*lnref_ptr);
    *lnref_ptr = NULL;
}


// create and initialize
 DListRef DList_new(DListItemDeallocator dealloc)
{
    DListRef lref = calloc(1, sizeof(DList));
    if(lref != NULL) {
        DList_init(lref, dealloc);
    }
    return lref;
}

// initialize a given block of memory as empty list
void DList_init( DListRef lref, DListItemDeallocator dealloc)
{
    ASSERT_NOT_NULL(lref);
    lref->count = 0;
    lref->head = NULL;
    lref->tail = NULL;
    lref->dealloc = dealloc;
}

// destroy the content including freeing any dynamic memory leaving a functioning empty list
void DList_destroy( DListRef lref)
{
    ASSERT_NOT_NULL(lref);
    DListItemDeallocator dealloc = lref->dealloc;
    DListNode* t = lref->head;
    for(;;) {
        // how to free the contained item
        if(t == NULL) {
            break;
        }
//        if(lref->dealloc != NULL) {
//            lref->dealloc(&(t->item));
//        }
        DListNode* tnext = t->forward;
        DListNode_dispose(lref, &t);
        t = tnext;
    }
    DList_init(lref, dealloc);
}

//free the entire list including invalidating the lref
void DList_dispose( DListRef* lref_ptr)
{
    ASSERT_NOT_NULL(*lref_ptr);
    DList_destroy(*lref_ptr);
    free((void*)*lref_ptr);
    *lref_ptr = NULL;
}
int DList_size(const DListRef lref)
{
    return lref->count;
}
void DList_display(const DListRef this)
{
    printf("List[%p] count: %d head %p tail %p\n", (void*)this, this->count, (void*)this->head, (void*)this->tail);
    DListNode* iter = this->head;
    while(iter != NULL) {
        printf("Node[%p] forward:%p backwards:%p  item:%p  %ld\n", (void*)iter, (void*)iter->forward, (void*)iter->backward, iter->item, (long)iter->item);
        DListNode* next = iter->forward;
        iter = next;
    }
}
 DListIterator DList_find( DListRef this, void* needle)
{
    printf("List[%p] count: %d head %p tail %p\n", (void*)this, this->count, (void*)this->head, (void*)this->tail);
    DListNode* iter = this->head;
    while(iter != NULL) {
        if(iter->item == needle) {
            return iter;
        }
        printf("Node[%p] forward:%p backwards:%p  item:%p  %ld\n", (void*)iter, (void*)iter->forward, (void*)iter->backward, iter->item, (long)iter->item);
        DListNode* next = iter->forward;
        iter = next;
    }
    return NULL;
}

// add to the front of the list
void DList_add_front( DListRef lref, void* content)
{
    ASSERT_NOT_NULL(lref);
    DListNode* lnref = DListNode_new(content, NULL, NULL);
    if(lref->count == 0) {
        lref->head = lnref;
        lref->tail = lnref;
        lref->count++;
    } else {
        lnref->forward = lref->head;
        lnref->backward = NULL;
        lref->head->backward = lnref;
        lref->head = lnref;
        lref->count++;
    }
}

// add to the back of the list
void DList_add_back( DListRef lref, void* content)
{
    ASSERT_NOT_NULL(lref);
    DListNode* lnref = DListNode_new(content, NULL, lref->head);
    if(lref->count == 0) {
        lref->tail = lnref;
        lref->head = lnref;
        lref->count++;
    } else {
        lnref->backward = lref->tail;
        lnref->forward = NULL;
        lref->tail->forward = lnref;
        lref->tail = lnref;
        lref->count++;
    }
}

// gets the item contained in the first list item without removing from list
void* DList_first(const DListRef lref)
{
    ASSERT_NOT_NULL(lref);
    if(lref->head == NULL)
        return NULL;
    return lref->head->item;
}

// gets the item contained in the first list item AND removes that item
void* DList_remove_first( DListRef lref)
{
    ASSERT_NOT_NULL(lref);
    if(lref->count == 0)
        return NULL;
    if(lref->count == 1) {
        lref->count--;
        void* content = lref->head->item;
        lref->head->item = NULL;
        DListNode_dispose(lref, &(lref->head));
        lref->head = NULL; lref->tail = NULL;
        return content;
    }
    DListNode* first = lref->head;
    lref->head = first->forward;
    lref->head->backward = NULL;
    void* content = first->item;
    first->forward = NULL;
    first->backward = NULL;
    first->item = NULL;
    DListNode_dispose(lref, &first);
    lref->count--;
    return content;
}

// gets the item contained in the last list item without removing from list
void* DList_last(const DListRef lref)
{
    ASSERT_NOT_NULL(lref);
    if(lref->tail == NULL) return NULL;
    return lref->tail->item;
}

// gets the item contained in the last list item AND removes that item
void* DList_remove_last( DListRef lref)
{
    ASSERT_NOT_NULL(lref);
    if(lref->count == 0 ) {
        return NULL;
    }
    if(lref->count == 1) {
        lref->count--;
        void* content = lref->head->item;
        lref->head->item = NULL;

        DListNode_dispose(lref, &(lref->head));
        lref->head = NULL; lref->tail = NULL;
        return content;
    }
    DListNode* last = lref->tail;
    lref->tail = last->backward;
    lref->tail->forward = NULL;
    void* content = last->item;
    last->forward = NULL;
    last->backward = NULL;
    last->item = NULL;

    DListNode_dispose(lref, &last);
    lref->count--;
    return content;
}

//gets an iterator for the list which initially will be pointing at the first Node in the list
 DListIterator DList_iterator(const DListRef lref)
{
    ASSERT_NOT_NULL(lref);
    return lref->head;
}

// moves the iterator on to the next Node on the list, returns NULL if goes off the end of the list
 DListIterator DList_itr_next(const DListRef lref, const DListIterator itr)
{
    ASSERT_NOT_NULL(lref);
    ASSERT_NOT_NULL(itr);
    return itr->forward;
}

// removes a list item pointed at by an iterator - invalidates the itr
// and if there is a dealloc function call it on the content of the list node
void DList_itr_remove( DListRef lref, DListIterator* itr_ptr)
{
    ASSERT_NOT_NULL(lref);
    ASSERT_NOT_NULL(itr_ptr);
    DListNode* itr = *itr_ptr;
    ASSERT_NOT_NULL(itr);
    if(lref->count == 0)
        return;
    else if(lref->count == 1 ) {
        assert(*itr_ptr == lref->head);
        lref->count = 0;
        lref->head = NULL;
        lref->tail = NULL;
        DListNode_dispose(lref, itr_ptr);
        return;
    }
    if(lref->head == *itr_ptr) {
        (*itr_ptr)->forward->backward = (*itr_ptr)->backward;
        lref->head = (*itr_ptr)->forward;
    } else if (lref->tail == *itr_ptr) {
        (*itr_ptr)->backward->forward = (*itr_ptr)->forward;
        lref->tail = (*itr_ptr)->backward;
    } else {
        (*itr_ptr)->forward->backward = (*itr_ptr)->backward;
        (*itr_ptr)->backward->forward = (*itr_ptr)->forward;
    }
    lref->count--;

    DListNode_dispose(lref, itr_ptr);
}

// gets the value of the item held in the Node pointed at by this iterator
void* DList_itr_unpack( DListRef lref, DListIterator itr)
{
    ASSERT_NOT_NULL(lref);
    ASSERT_NOT_NULL(itr);
    return itr->item;
}
