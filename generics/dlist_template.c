//opaque type representing list
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <http_in_c/common/list.h>
#include <http_in_c/common/utils.h>

struct PREFIX(Node_s) {
    PREFIX(Node)* forward;
    PREFIX(Node)* backward;
    TYPE* item;
};

struct PREFIX(_s) {
    int count;
    PREFIX(Node)* head;
    PREFIX(Node)* tail;
    ListItemDeallocator dealloc;
};

PREFIX(Node)* PREFIX(Node_new)(TYPE* content, PREFIX(Node)* prev, PREFIX(Node)* next)
{
    PREFIX(Node)* lnref = malloc(sizeof(PREFIX(Node)));
    lnref->item = content;
    lnref->forward = next;
    lnref->backward = prev;
}
void PREFIX(Node_dispose)(PREFIX(Ref) lref, PREFIX(Node)** lnref_ptr)
{
    ASSERT_NOT_NULL(lref);
    ASSERT_NOT_NULL(lnref_ptr);
    PREFIX(Node)* lnref = *lnref_ptr;
    if(lref->dealloc != NULL) {
        if(lnref->item != NULL)
            TYPED(_free)(lnref->item);
            lnref->item = NULL;
    }
    free((void*)*lnref_ptr);
    *lnref_ptr = NULL;
}


// create and initialize
PREFIX(Ref) PREFIX(_new)()
{
    PREFIX(Ref) lref = malloc(sizeof(PREFIX()));
    if(lref != NULL) {
        PREFIX(_init)(lref);
    }
    return lref;
}

// initialize a given block of memory as empty list
void PREFIX(_init)(PREFIX(Ref) lref)
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
void PREFIX(_destroy)(PREFIX(Ref) lref)
{
    ASSERT_NOT_NULL(lref);
    ListItemDeallocator dealloc = lref->dealloc;
    PREFIX(Node)* t = lref->head;
    for(;;) {
        // how to free the contained item
        if(t == NULL) {
            break;
        }
        PREFIX(Node)* tnext = t->forward;
        PREFIX(Node_dispose)(lref, &t);
        t = tnext;
    }
    PREFIX(_init)(lref);
}

//free the entire list including invalidating the lref
void PREFIX(_dispose)(PREFIX(Ref)* lref_ptr)
{
    ASSERT_NOT_NULL(*lref_ptr);
    List_destroy((ListRef)*lref_ptr);
    free((void*)*lref_ptr);
    *lref_ptr = NULL;
}
int PREFIX(_size)(const PREFIX(Ref) lref)
{
    return lref->count;
}
void PREFIX(_display)(const PREFIX(Ref) this)
{
    printf("List[%p] count: %d head %p tail %p\n", (void*)this, this->count, (void*)this->head, (void*)this->tail);
    PREFIX(Node)* iter = this->head;
    while(iter != NULL) {
        printf("Node[%p] forward:%p backwards:%p  item:%p  %ld\n", (void*)iter, (void*)iter->forward, (void*)iter->backward, iter->item, (long)iter->item);
        TYPED(_display)(iter->item);
        PREFIX(Node)* next = iter->forward;
        iter = next;
    }
}
PREFIX(Iterator) PREFIX(_find)(PREFIX(Ref) this, TYPE* needle)
{
    printf("List[%p] count: %d head %p tail %p\n", (void*)this, this->count, (void*)this->head, (void*)this->tail);
    PREFIX(Node)* iter = this->head;
    while(iter != NULL) {
        if(iter->item == needle) {
            return iter;
        }
        printf("Node[%p] forward:%p backwards:%p  item:%p  %ld\n", (void*)iter, (void*)iter->forward, (void*)iter->backward, iter->item, (long)iter->item);
        PREFIX(Node)* next = iter->forward;
        iter = next;
    }
    return NULL;
}

// add to the front of the list
void PREFIX(_add_front)(PREFIX(Ref) lref, TYPE* content)
{
    ASSERT_NOT_NULL(lref);
    PREFIX(Node)* lnref = PREFIX(Node_new)(content, NULL, NULL);
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
void PREFIX(_add_back)(PREFIX(Ref) lref, TYPE* content)
{
    ASSERT_NOT_NULL(lref);
    PREFIX(Node)* lnref = PREFIX(Node_new)(content, NULL, lref->head);
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
TYPE* PREFIX(_first)(const PREFIX(Ref) lref)
{
    ASSERT_NOT_NULL(lref);
    if(lref->head == NULL)
        return NULL;
    return lref->head->item;
}

// gets the item contained in the first list item AND removes that item
TYPE* PREFIX(_remove_first)(PREFIX(Ref) lref)
{
    ASSERT_NOT_NULL(lref);
    if(lref->count == 0)
        return NULL;
    if(lref->count == 1) {
        lref->count--;
        void* content = lref->head->item;
        lref->head->item = NULL;
        PREFIX(Node_dispose)(lref, &(lref->head));
        lref->head = NULL; lref->tail = NULL;
        return content;
    }
    PREFIX(Node)* first = lref->head;
    lref->head = first->forward;
    lref->head->backward = NULL;
    void* content = first->item;
    first->forward = NULL;
    first->backward = NULL;
    first->item = NULL;
    PREFIX(Node_dispose)(lref, &first);
    lref->count--;
    return content;
}

// gets the item contained in the last list item without removing from list
TYPE* PREFIX(_last)(const PREFIX(Ref) lref)
{
    ASSERT_NOT_NULL(lref);
    if(lref->tail == NULL) return NULL;
    return lref->tail->item;
}

// gets the item contained in the last list item AND removes that item
TYPE* PREFIX(_remove_last)(PREFIX(Ref) lref)
{
    ASSERT_NOT_NULL(lref);
    if(lref->count == 0 ) {
        return NULL;
    }
    if(lref->count == 1) {
        lref->count--;
        void* content = lref->head->item;
        lref->head->item = NULL;

        PREFIX(Node_dispose)(lref, &(lref->head));
        lref->head = NULL; lref->tail = NULL;
        return content;
    }
    PREFIX(Node)* last = lref->tail;
    lref->tail = last->backward;
    lref->tail->forward = NULL;
    void* content = last->item;
    last->forward = NULL;
    last->backward = NULL;
    last->item = NULL;

    PREFIX(Node_dispose)(lref, &last);
    lref->count--;
    return content;
}

//gets an iterator for the list which initially will be pointing at the first Node in the list
PREFIX(Iterator) PREFIX(_iterator)(const PREFIX(Ref) lref)
{
    ASSERT_NOT_NULL(lref);
    return lref->head;
}

// moves the iterator on to the next Node on the list, returns NULL if goes off the end of the list
PREFIX(Iterator) PREFIX(_itr_next)(const PREFIX(Ref) lref, const PREFIX(Iterator) itr)
{
    ASSERT_NOT_NULL(lref);
    ASSERT_NOT_NULL(itr);
    return itr->forward;
}

// removes a list item pointed at by an iterator - invalidates the itr
// and if there is a dealloc function call it on the content of the list node
void PREFIX(_itr_remove)(PREFIX(Ref) lref, PREFIX(Iterator)* itr_ptr)
{
    ASSERT_NOT_NULL(lref);
    ASSERT_NOT_NULL(itr_ptr);
    PREFIX(Node)* itr = *itr_ptr;
    ASSERT_NOT_NULL(itr);
    if(lref->count == 0)
        return;
    else if(lref->count == 1 ) {
        assert(*itr_ptr == lref->head);
        lref->count = 0;
        lref->head = NULL;
        lref->tail = NULL;
        PREFIX(Node_dispose)(lref, itr_ptr);
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

    PREFIX(Node_dispose)(lref, itr_ptr);
}

// gets the value of the item held in the Node pointed at by this iterator
TYPE* PREFIX(_itr_unpack)(PREFIX(Ref) lref, PREFIX(Iterator) itr)
{
    ASSERT_NOT_NULL(lref);
    ASSERT_NOT_NULL(itr);
    return itr->item;
}
