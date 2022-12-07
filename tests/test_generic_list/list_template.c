//opaque type representing list
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <c_http/common/utils.h>

struct TYPED(ListNode_s) {
    TYPED(ListNode)* forward;
    TYPED(ListNode)* backward;
    void* item;
};

struct TYPED(List_s) {
    int count;
    TYPED(ListNode)* head;
    TYPED(ListNode)* tail;
    ListItemDeallocator dealloc;
};

TYPED(ListNode)* TYPED(ListNode_new)(void* content, TYPED(ListNode)* prev, TYPED(ListNode)* next)
{
    TYPED(ListNode)* lnref = malloc(sizeof(TYPED(ListNode)));
    lnref->item = content;
    lnref->forward = next;
    lnref->backward = prev;
}
void TYPED(ListNode_dispose)(TYPED(ListRef) lref, TYPED(ListNode)** lnref_ptr)
{
    ASSERT_NOT_NULL(lref);
    ASSERT_NOT_NULL(lnref_ptr);
    TYPED(ListNode)* lnref = *lnref_ptr;
    if(lref->dealloc != NULL) {
        if(lnref->item != NULL)
            TYPED(_free)(lnref->item);
            lnref->item = NULL;
    }
    free((void*)*lnref_ptr);
    *lnref_ptr = NULL;
}


// create and initialize
TYPED(ListRef) TYPED(List_new)()
{
    TYPED(ListRef) lref = malloc(sizeof(TYPED(List)));
    if(lref != NULL) {
        TYPED(List_init)(lref);
    }
    return lref;
}

// initialize a given block of memory as empty list
void TYPED(List_init)(TYPED(ListRef) lref)
{
    ASSERT_NOT_NULL(lref);
    lref->count = 0;
    lref->head = NULL;
    lref->tail = NULL;
    lref->dealloc = &(TYPED(_dealloc));
    void(*x1)(TYPE*) = &(TYPED(_free)); // test to ensure TYPE has a free function
    void(*x2)(void**) = &(TYPED(_dealloc)); // test to ensure TYPE has a deallocator
    void(*x3)(TYPE*) = &(TYPED(_display)); // test to ensure TYPE has a display
}

// destroy the content including freeing any dynamic memory leaving a functioning empty list
void TYPED(List_destroy)(TYPED(ListRef) lref)
{
    ASSERT_NOT_NULL(lref);
    ListItemDeallocator dealloc = lref->dealloc;
    TYPED(ListNode)* t = lref->head;
    for(;;) {
        // how to free the contained item
        if(t == NULL) {
            break;
        }
        TYPED(ListNode)* tnext = t->forward;
        TYPED(ListNode_dispose)(lref, &t);
        t = tnext;
    }
    TYPED(List_init)(lref);
}

//free the entire list including invalidating the lref
void TYPED(List_dispose)(TYPED(ListRef)* lref_ptr)
{
    ASSERT_NOT_NULL(*lref_ptr);
    List_destroy(*lref_ptr);
    free((void*)*lref_ptr);
    *lref_ptr = NULL;
}
int TYPED(List_size)(const TYPED(ListRef) lref)
{
    return lref->count;
}
void TYPED(List_display)(const TYPED(ListRef) this)
{
    printf("List[%p] count: %d head %p tail %p\n", (void*)this, this->count, (void*)this->head, (void*)this->tail);
    TYPED(ListNode)* iter = this->head;
    while(iter != NULL) {
        printf("Node[%p] forward:%p backwards:%p  item:%p  %ld\n", (void*)iter, (void*)iter->forward, (void*)iter->backward, iter->item, (long)iter->item);
        TYPED(_display)(iter->item);
        TYPED(ListNode)* next = iter->forward;
        iter = next;
    }
}
TYPED(ListIterator) TYPED(List_find)(TYPED(ListRef) this, void* needle)
{
    printf("List[%p] count: %d head %p tail %p\n", (void*)this, this->count, (void*)this->head, (void*)this->tail);
    TYPED(ListNode)* iter = this->head;
    while(iter != NULL) {
        if(iter->item == needle) {
            return iter;
        }
        printf("Node[%p] forward:%p backwards:%p  item:%p  %ld\n", (void*)iter, (void*)iter->forward, (void*)iter->backward, iter->item, (long)iter->item);
        TYPED(ListNode)* next = iter->forward;
        iter = next;
    }
    return NULL;
}

// add to the front of the list
void TYPED(List_add_front)(TYPED(ListRef) lref, void* content)
{
    ASSERT_NOT_NULL(lref);
    TYPED(ListNode)* lnref = TYPED(ListNode_new)(content, NULL, NULL);
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
void TYPED(List_add_back)(TYPED(ListRef) lref, void* content)
{
    ASSERT_NOT_NULL(lref);
    TYPED(ListNode)* lnref = TYPED(ListNode_new)(content, NULL, lref->head);
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
void* TYPED(List_first)(const TYPED(ListRef) lref)
{
    ASSERT_NOT_NULL(lref);
    if(lref->head == NULL)
        return NULL;
    return lref->head->item;
}

// gets the item contained in the first list item AND removes that item
void* TYPED(List_remove_first)(TYPED(ListRef) lref)
{
    ASSERT_NOT_NULL(lref);
    if(lref->count == 0)
        return NULL;
    if(lref->count == 1) {
        lref->count--;
        void* content = lref->head->item;
        lref->head->item = NULL;
        TYPED(ListNode_dispose)(lref, &(lref->head));
        lref->head = NULL; lref->tail = NULL;
        return content;
    }
    TYPED(ListNode)* first = lref->head;
    lref->head = first->forward;
    lref->head->backward = NULL;
    void* content = first->item;
    first->forward = NULL;
    first->backward = NULL;
    first->item = NULL;
    TYPED(ListNode_dispose)(lref, &first);
    lref->count--;
    return content;
}

// gets the item contained in the last list item without removing from list
void* TYPED(List_last)(const TYPED(ListRef) lref)
{
    ASSERT_NOT_NULL(lref);
    if(lref->tail == NULL) return NULL;
    return lref->tail->item;
}

// gets the item contained in the last list item AND removes that item
void* TYPED(List_remove_last)(TYPED(ListRef) lref)
{
    ASSERT_NOT_NULL(lref);
    if(lref->count == 0 ) {
        return NULL;
    }
    if(lref->count == 1) {
        lref->count--;
        void* content = lref->head->item;
        lref->head->item = NULL;

        TYPED(ListNode_dispose)(lref, &(lref->head));
        lref->head = NULL; lref->tail = NULL;
        return content;
    }
    TYPED(ListNode)* last = lref->tail;
    lref->tail = last->backward;
    lref->tail->forward = NULL;
    void* content = last->item;
    last->forward = NULL;
    last->backward = NULL;
    last->item = NULL;

    TYPED(ListNode_dispose)(lref, &last);
    lref->count--;
    return content;
}

//gets an iterator for the list which initially will be pointing at the first Node in the list
TYPED(ListIterator) TYPED(List_iterator)(const TYPED(ListRef) lref)
{
    ASSERT_NOT_NULL(lref);
    return lref->head;
}

// moves the iterator on to the next Node on the list, returns NULL if goes off the end of the list
TYPED(ListIterator) TYPED(List_itr_next)(const TYPED(ListRef) lref, const TYPED(ListIterator) itr)
{
    ASSERT_NOT_NULL(lref);
    ASSERT_NOT_NULL(itr);
    return itr->forward;
}

// removes a list item pointed at by an iterator - invalidates the itr
// and if there is a dealloc function call it on the content of the list node
void TYPED(List_itr_remove)(TYPED(ListRef) lref, TYPED(ListIterator)* itr_ptr)
{
    ASSERT_NOT_NULL(lref);
    ASSERT_NOT_NULL(itr_ptr);
    TYPED(ListNode)* itr = *itr_ptr;
    ASSERT_NOT_NULL(itr);
    if(lref->count == 0)
        return;
    else if(lref->count == 1 ) {
        assert(*itr_ptr == lref->head);
        lref->count = 0;
        lref->head = NULL;
        lref->tail = NULL;
        TYPED(ListNode_dispose)(lref, itr_ptr);
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

    TYPED(ListNode_dispose)(lref, itr_ptr);
}

// gets the value of the item held in the Node pointed at by this iterator
void* TYPED(List_itr_unpack)(TYPED(ListRef) lref, TYPED(ListIterator) itr)
{
    ASSERT_NOT_NULL(lref);
    ASSERT_NOT_NULL(itr);
    return itr->item;
}
