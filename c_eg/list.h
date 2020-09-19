#ifndef c_ceg_list_h
#define c_ceg_list_h

// type of data to be held in the list
typedef void* ListItem;

//opaque type representing list
struct List_s;
typedef struct List_s List, *ListRef;

//Internal - type used to build list
struct ListNode_s;

typedef struct ListNode_s ListNode, *ListNodeRef;


// a function that knows how to free whatever the void* content of node is pointing at
typedef void(*ListItemDeallocator)(void*);


// create and initialize
ListRef List_new(ListItemDeallocator dealloc);

// initialize a given block of memory as empty list
void List_init(ListRef lref, ListItemDeallocator dealloc);

// destroy the content including freeing any dynamic memory leaving a functioning empty list
void List_destroy(ListRef lref);

//free the entire list including invalidating the lref
void List_free(ListRef* lref_ptr);

//returns number of nodes on LIst
int List_size(ListRef lref);

// add to the end of the list
void List_add_back(ListRef lref, void*);

// add to the front of the list
void List_add_front(ListRef lref, void*);

// gets the item contained in the first list item without removing from list
void* List_first(ListRef lref);

// gets the item contained in the first list item AND removes that item
void* List_remove_first(ListRef lref);

// gets the item contained in the last list item without removing from list
void* List_last(ListRef lref);

// gets the item contained in the last list item AND removes that item
void* List_remove_last(ListRef lref);

//gets an iterator for the list which initially will be pointing at the first Node in the list
ListNodeRef List_iterator(ListRef lref);

// moves the iterator on to the next Node on the list, returns NULL if goes off the end of the list
ListNodeRef List_itr_next(ListRef lref, ListNodeRef itr);

// removes a list item pointed at by an iterator, frees that item and invalidates the iterator
// and sets to NULL the variable pointing to it
void List_itr_remove(ListRef lref, ListNodeRef* itr);

// gets the value of the item held in the Node pointed at by this iterator
void* List_itr_unpack(ListRef lref, ListNodeRef itr);
#ifdef MNMNM

#endif
#endif