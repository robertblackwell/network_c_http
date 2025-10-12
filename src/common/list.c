//opaque type representing list
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <common/alloc.h>
#include <common/list.h>
#include <common/utils.h>
//Internal - type used to build list

struct ListNode_s {
    ListNode* forward;
    ListNode* backward;
    void* item;
};

struct List_s {
    int count;
    ListNode* head;
    ListNode* tail;
};

ListNode* ListNode_new(void* content, ListNode* prev, ListNode* next)
{
    ListNode* lnref = eg_alloc(sizeof(ListNode));
    lnref->item = content;
    lnref->forward = next;
    lnref->backward = prev;
    return lnref;
}
void ListNode_free(ListNode* this)
{
    free(this);
}
void ListNode_dispose(ListRef lref, ListNode** lnref_ptr)
{
    assert(0);
//    ASSERT_NOT_NULL(lref);
//    ASSERT_NOT_NULL(lnref_ptr);
//    ListNode* lnref = *lnref_ptr;
//    if(lref->dealloc != NULL) {
//        if(lnref->item != NULL)
//            lref->dealloc(&(lnref->item));
//    }
//    eg_free((void*)*lnref_ptr);
//    *lnref_ptr = NULL;
}


// create and initialize
ListRef List_new()
{
    ListRef lref = malloc(sizeof(List));
    if(lref != NULL) {
        List_init(lref);
    }
    return lref;
}

// initialize a given block of memory as empty list
void List_init(ListRef lref)
{
    ASSERT_NOT_NULL(lref);
    lref->count = 0;
    lref->head = NULL;
    lref->tail = NULL;
}

// destroy the content including freeing any dynamic memory leaving a functioning empty list
void List_safe_deinit(ListRef this, void(*item_free)(void*))
{
    ASSERT_NOT_NULL(this);
    if(this->count == 0) {
        return;
    }
    assert(item_free != NULL);
    ListNode* t = this->head;
    for(;;) {
        // how to free the contained item
        if(t == NULL) {
            break;
        }
        item_free((t->item));
        ListNode* tnext = t->forward;
        ListNode_free(t);
        t = tnext;
    }
    this->count = 0;
    this->head = NULL;
    this->tail = NULL;
}
void List_free(ListRef lref)
{
    ASSERT_NOT_NULL(lref);
    assert(lref->count == 0);
    free(lref);
}
void List_safe_free(ListRef this, void(*item_free)(void*))
{
    List_safe_deinit(this, item_free);
    List_free(this);
}
//free the entire list including invalidating the lref
//void List_dispose(ListRef* lref_ptr)
//{
//    ASSERT_NOT_NULL(*lref_ptr);
//    List_destroy(*lref_ptr);
//    free((void*)*lref_ptr);
//    *lref_ptr = NULL;
//}
int List_size(const ListRef lref)
{
    return lref->count;
}
void List_display(const ListRef this)
{
    printf("List[%p] count: %d head %p tail %p\n", (void*)this, this->count, (void*)this->head, (void*)this->tail);
    ListNode* iter = this->head;
    while(iter != NULL) {
        printf("Node[%p] forward:%p backwards:%p  item:%p  %ld\n", (void*)iter, (void*)iter->forward, (void*)iter->backward, iter->item, (long)iter->item);
        ListNode* next = iter->forward;
        iter = next;
    }
}
ListIterator List_find(ListRef this, void* needle)
{
//    printf("List[%p] count: %d head %p tail %p\n", (void*)this, this->count, (void*)this->head, (void*)this->tail);
    ListNode* iter = this->head;
    while(iter != NULL) {
        if(iter->item == needle) {
            return iter;
        }
//        printf("Node[%p] forward:%p backwards:%p  item:%p  %ld\n", (void*)iter, (void*)iter->forward, (void*)iter->backward, iter->item, (long)iter->item);
        ListNode* next = iter->forward;
        iter = next;
    }
    return NULL;
}

// add to the front of the list
void List_add_front(ListRef lref, void* content)
{
    ASSERT_NOT_NULL(lref);
    ListNode* lnref = ListNode_new(content, NULL, NULL);
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
void List_add_back(ListRef lref, void* content)
{
    ASSERT_NOT_NULL(lref);
    ListNode* lnref = ListNode_new(content, NULL, lref->head);
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
void* List_first(const ListRef lref)
{
    ASSERT_NOT_NULL(lref);
    if(lref->head == NULL)
        return NULL;
    return lref->head->item;
}

// gets the item contained in the first list item AND removes that item
void* List_remove_first(ListRef lref)
{
    assert(lref != NULL);
    if(lref->count == 0)
        return NULL;
    if(lref->count == 1) {
        lref->count--;
        void* content = lref->head->item;
        lref->head->item = NULL;
        ListNode_free(lref->head);
        lref->head = NULL; lref->tail = NULL;
        return content;
    }
    ListNode* first = lref->head;
    lref->head = first->forward;
    lref->head->backward = NULL;
    void* content = first->item;
    first->forward = NULL;
    first->backward = NULL;
    first->item = NULL;
    ListNode_free(first);
    lref->count--;
    return content;
}

// gets the item contained in the last list item without removing from list
void* List_last(const ListRef lref)
{
    ASSERT_NOT_NULL(lref);
    if(lref->tail == NULL) return NULL;
    return lref->tail->item;
}

// gets the item contained in the last list item AND removes that item
void* List_remove_last(ListRef lref)
{
    ASSERT_NOT_NULL(lref);
    if(lref->count == 0 ) {
        return NULL;
    }
    if(lref->count == 1) {
        lref->count--;
        void* content = lref->head->item;
        lref->head->item = NULL;

        ListNode_free(lref->head    );
        lref->head = NULL; lref->tail = NULL;
        return content;
    }
    ListNode* last = lref->tail;
    lref->tail = last->backward;
    lref->tail->forward = NULL;
    void* content = last->item;
    last->forward = NULL;
    last->backward = NULL;
    last->item = NULL;

    ListNode_free(last);
    lref->count--;
    return content;
}

//gets an iterator for the list which initially will be pointing at the first Node in the list
ListIterator List_iterator(const ListRef lref)
{
    ASSERT_NOT_NULL(lref);
    return lref->head;
}

// moves the iterator on to the next Node on the list, returns NULL if goes off the end of the list
ListIterator List_itr_next(const ListRef lref, const ListIterator itr)
{
    ASSERT_NOT_NULL(lref);
    ASSERT_NOT_NULL(itr);
    return itr->forward;
}

// removes a list item pointed at by an iterator - invalidates the itr
// and if there is a dealloc function call it on the content of the list node
void List_itr_remove(ListRef lref, ListIterator* itr_ptr)
{
    ASSERT_NOT_NULL(lref);
    ASSERT_NOT_NULL(itr_ptr);
    ListNode* itr = *itr_ptr;
    ASSERT_NOT_NULL(itr);
    if(lref->count == 0)
        return;
    else if(lref->count == 1 ) {
        assert(*itr_ptr == lref->head);
        lref->count = 0;
        lref->head = NULL;
        lref->tail = NULL;
        ListNode_free(*itr_ptr);
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

    ListNode_free(*itr_ptr);
}

// gets the value of the item held in the Node pointed at by this iterator
void* List_itr_unpack(ListRef lref, ListIterator itr)
{
    ASSERT_NOT_NULL(lref);
    ASSERT_NOT_NULL(itr);
    return itr->item;
}
