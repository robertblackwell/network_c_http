//opaque type representing list
#include <stdlib.h>
#include <assert.h>
#include <c_eg/list.h>
#include <c_eg/utils.h>
//Internal - type used to build list

struct ListNode_s {
    ListNodeRef forward;
    ListNodeRef backward;
    void* item;
};

struct List_s {
    int count;
    ListNodeRef head;
    ListNodeRef tail;
    ListItemDeallocator dealloc;
};

ListNodeRef ListNode_new(void* content, ListNodeRef prev, ListNodeRef next)
{
    ListNodeRef lnref = malloc(sizeof(ListNode));
    lnref->item = content;
    lnref->forward = next;
    lnref->backward = prev;
}
void ListNode_free(ListNodeRef* lnref_ptr)
{
    ASSERT_NOT_NULL(lnref_ptr);
    free((void*)*lnref_ptr);
    *lnref_ptr = NULL;
}


// create and initialize
ListRef List_new(ListItemDeallocator dealloc)
{
    ListRef lref = malloc(sizeof(List));
    if(lref != NULL) {
        List_init(lref, dealloc);
    }
    return lref;
}

// initialize a given block of memory as empty list
void List_init(ListRef lref, ListItemDeallocator dealloc)
{
    ASSERT_NOT_NULL(lref);
    lref->count = 0;
    lref->head = NULL;
    lref->tail = NULL;
    lref->dealloc = dealloc;
}

// destroy the content including freeing any dynamic memory leaving a functioning empty list
void List_destroy(ListRef lref)
{
    ASSERT_NOT_NULL(lref);
    ListItemDeallocator dealloc = lref->dealloc;
    ListNodeRef t = lref->head;
    for(;;) {
        // how to free the contained item
        if(t == NULL) {
            break;
        }
        if(lref->dealloc != NULL) {
            lref->dealloc(t->item);
        }
        ListNodeRef tnext = t->forward;
        ListNode_free(&t);
        t = tnext;
    }
    List_init(lref, dealloc);
}

//free the entire list including invalidating the lref
void List_free(ListRef* lref_ptr)
{
    ASSERT_NOT_NULL(*lref_ptr);
    List_destroy(*lref_ptr);
    free((void*)*lref_ptr);
    *lref_ptr = NULL;
}
int List_size(ListRef lref)
{
    return lref->count;
}
// add to the front of the list
void List_add_front(ListRef lref, void* content)
{
    ASSERT_NOT_NULL(lref);
    ListNodeRef lnref = ListNode_new(content, lref->tail, NULL);
    if(lref->count == 0) {
        lref->head = lnref;
        lref->tail = lnref;
        lref->count++;
    } else {
        lnref->forward = lref->head;
        lnref->backward = NULL;
        lref->head->backward = lnref;
        lref->tail = lnref;
        lref->count++;
    }
}

// add to the back of the list
void List_add_back(ListRef lref, void* content)
{
    ASSERT_NOT_NULL(lref);
    ListNodeRef lnref = ListNode_new(content, NULL, lref->head);
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
void* List_first(ListRef lref)
{
    ASSERT_NOT_NULL(lref);
    return lref->head->item;
}

// gets the item contained in the first list item AND removes that item
void* List_remove_first(ListRef lref)
{
    ASSERT_NOT_NULL(lref);
    if(lref->count == 0)
        return NULL;
    if(lref->count == 1) {
        lref->count--;
        void* content = lref->head->item;
        ListNode_free(&(lref->head));
        lref->head = NULL; lref->tail = NULL;
        return content;
    }
    ListNodeRef first = lref->head;
    lref->head = first->forward;
    lref->head->backward = NULL;
    void* content = first->item;
    first->forward = NULL;
    first->backward = NULL;
    ListNode_free(&first);
    lref->count--;
    return content;
}

// gets the item contained in the last list item without removing from list
void* List_last(ListRef lref)
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
        ListNode_free(&(lref->head));
        lref->head = NULL; lref->tail = NULL;
        return content;
    }
    ListNodeRef last = lref->tail;
    lref->tail = last->backward;
    lref->tail->forward = NULL;
    void* content = last->item;
    last->forward = NULL; last->backward = NULL;
    ListNode_free(&last);
    lref->count--;
    return content;
}

//gets an iterator for the list which initially will be pointing at the first Node in the list
ListNodeRef List_iterator(ListRef lref)
{
    ASSERT_NOT_NULL(lref);
    return lref->head;
}

// moves the iterator on to the next Node on the list, returns NULL if goes off the end of the list
ListNodeRef List_itr_next(ListRef lref, ListNodeRef itr)
{
    ASSERT_NOT_NULL(lref);
    ASSERT_NOT_NULL(itr);
    return itr->forward;
}

// removes a list item pointed at by an iterator - invalidates the itr
void List_itr_remove(ListRef lref, ListNodeRef* itr_ptr)
{
    ASSERT_NOT_NULL(lref);
    ListNodeRef itr = *itr_ptr;
    ASSERT_NOT_NULL(itr);
    if(lref->count == 0)
        return;
    else if(lref->count == 1 ) {
        assert(*itr_ptr == lref->head);
        lref->count = 0;
        lref->head = NULL;
        lref->tail = NULL;
        ListNode_free(itr_ptr);
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
    ListNode_free(itr_ptr);
}

// gets the value of the item held in the Node pointed at by this iterator
void* List_itr_unpack(ListRef lref, ListNodeRef itr)
{
    ASSERT_NOT_NULL(lref);
    ASSERT_NOT_NULL(itr);
    return itr->item;
}
