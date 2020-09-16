//opaque type representing list
#include <stdlib.h>
#include <assert.h>
#include <c_eg/list.h>
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
void ListNode_free(ListNodeRef lnref)
{
    free((void*)lnref);
}


// create and initialize
ListRef List_new(ListItemDeallocator dealloc)
{
    ListRef lref = malloc(sizeof(List));
    List_init(lref, dealloc);
    return lref;
}

// initialize a given block of memory as empty list
void List_init(ListRef lref, ListItemDeallocator dealloc)
{
    lref->count = 0;
    lref->head = NULL;
    lref->tail = NULL;
    lref->dealloc = dealloc;
}

// destroy the content including freeing any dynamic memory leaving a functioning empty list
void List_destroy(ListRef lref)
{
    ListItemDeallocator dealloc = lref->dealloc;
    ListNodeRef t = lref->head;
    for(;;) {
        // how to free the contained item
        lref->dealloc(t->item);
        ListNodeRef tnext = t->forward;
        ListNode_free(t);
        t = tnext;
        if(t == NULL)
            break;
    }
    List_init(lref, dealloc);
}

//free the entire list including invalidating the lref
void List_free(ListRef lref)
{
    List_destroy(lref);
    free((void*)lref);
}
size_t List_size(ListRef lref)
{
    return lref->count;
}
// add to the end of the list
void List_add_back(ListRef lref, void* content)
{
    ListNodeRef lnref = ListNode_new(content, lref->tail, NULL);
    if(lref->count == 0) {
        lref->head = lnref;
    }
    lref->tail = lnref;
    lref->count++;
}

// add to the front of the list
void List_add_front(ListRef lref, void* content)
{
    ListNodeRef lnref = ListNode_new(content, NULL, lref->head);
    if(lref->count == 0) {
        lref->tail = lnref;
    }
    lref->head = lnref;
    lref->count++;
}

// gets the item contained in the first list item without removing from list
void* List_first(ListRef lref)
{
    return lref->head->item;
}

// gets the item contained in the first list item AND removes that item
void* List_remove_first(ListRef lref)
{
    if(lref->count == 0)
        return NULL;
    if(lref->count == 1) {
        lref->count--;
        void* content = lref->head->item;
        ListNode_free(lref->head);
        lref->head = NULL; lref->tail = NULL;
        return content;
    }
    ListNodeRef first = lref->head;
    lref->head = first->forward;
    lref->head->backward = NULL;
    void* content = first->item;
    first->forward = NULL;
    first->backward = NULL;
    ListNode_free(first);
    lref->count--;
    return content;
}

// gets the item contained in the last list item without removing from list
void* List_last(ListRef lref)
{
    return lref->tail->item;
}

// gets the item contained in the last list item AND removes that item
void* List_remove_last(ListRef lref)
{
    if(lref->count == 0 ) {
        return NULL;
    }
    if(lref->count == 1) {
        lref->count--;
        void* content = lref->head->item;
        ListNode_free(lref->head);
        lref->head = NULL; lref->tail = NULL;
        return content;
    }
    ListNodeRef last = lref->tail;
    lref->tail = last->backward;
    lref->tail->forward = NULL;
    void* content = last->item;
    last->forward = NULL; last->backward = NULL;
    ListNode_free(last);
    lref->count--;
    return content;
}

//gets an iterator for the list which initially will be pointing at the first Node in the list
ListNodeRef List_iterator(ListRef lref)
{
    return lref->head;
}

// moves the iterator on to the next Node on the list, returns NULL if goes off the end of the list
ListNodeRef List_itr_next(ListRef lref, ListNodeRef itr)
{
    assert(itr != NULL);
    return itr->forward;
}

// removes a list item pointed at by an iterator - invalidates the itr
void List_itr_remove(ListRef lref, ListNodeRef itr)
{
    if(itr->backward != NULL)
        itr->backward->forward = itr->forward;

    if(itr->forward != NULL)
        itr->forward->backward = itr->backward;

    ListNode_free(itr);
}

// gets the value of the item held in the Node pointed at by this iterator
void* List_itr_unpack(ListRef lref, ListNodeRef itr)
{
    assert(itr != NULL);
    return itr->item;
}
#ifdef BVBVB

#endif