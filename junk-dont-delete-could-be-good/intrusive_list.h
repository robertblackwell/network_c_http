#ifndef rbl_intrusive_list_h
#define rbl_intrusive_list_h

/**
 * An intrusive list is one where the elements of the list consciously hold the list pointers at the
 * front of their objects.
 *
 * So all types that intend to be elements of an intrusive list should be declared like this:
 *
 * struct {
 *      IntrusiveListItemHeader;
 *      // Then the other firslds that this struct requires
        type1 field1
        type2 field2
        ...
        ...
        typen fieldn
 * } IntrusiveListItem;
 *
 * An intrusive list has two advantages over our definition of a "standard" list:
 *
 * -    the list code does not have to do any memory management
 * -    potentially reduces the number of memory allocation/deallocation calls.
 *
 *
 * @addtogroup group_list
 * @{
*/

struct IntrusiveListNode_s;
typedef struct IntrusiveListNode_s IntrusiveListNode;
typedef struct IntrusiveListNode_s IntrusiveListHeader;
typedef IntrusiveListNode* IntrusiveListIterator, *IntrusiveListIter;
struct IntrusiveListNode_s {
    RBL_TAG    tag;
    IntrusiveListNode* forward;
    IntrusiveListNode* backward;
    RBL_TAG end_tag;
};


///
/// List is an opaque type. Can hold anonamous objects of type void*
/// Typically used as the base for lists of specific types by casting
/// MyType*/MyTypeRef to void*.
/// But can also hold int, long, char etc
///
struct IntrusiveList_s;
typedef struct IntrusiveList_s IntrusiveList, *IntrusiveListRef;


///
/// WARNING - THIS FUNCTION ALLOCATES MEMORY
///
///
/// Dynamically allocates a new List and return a ListRef, reference to the opaque List structure.
///
/// \param dealloc ListItemDeallocator
/// \return ListRef
///
IntrusiveListRef IntrusiveList_new();

///
/// Initializes a list structure for which the memory has already been acquired.
/// Typically for local variables of type List, called as:
///
///     List mylist;
///     List_init(&mylist, mydealloc_function);
///
/// \param lref    ListRef or ListRef
/// \param dealloc ListItemDeallocator
///
void IntrusiveList_init(IntrusiveListRef lref);

///
/// De-init an existing List instance
/// In particular this empties the list and if appropriate deallocates
/// all the nodes of the list and applies dealloc function to each list nodes
/// void* content.
/// But does not deallocate the memory for the list structure itself.
///
void IntrusiveList_destroy(IntrusiveListRef lref);

///
/// Frees all memory associated with the list.
///
/// NOTE: the ListRef* lref_ptr argument maybe a little unusual. This function is used as follows:
///
///   ListRef mylist = List_new(mydeallocator_function);
///
///   ..... do stuff with the list
///
///   List_dispose(&mylist); Note: the & on the ListRef
///                       Note also after the return from List_dispose() mylist == NULL
///
///
void IntrusiveList_dispose(IntrusiveListRef *lref_adr);

///
/// Prints a dump of the list to stdout
///
/// \param this ListRef Note the name
///
void IntrusiveList_display(const IntrusiveListRef this);

///
/// Returns number of nodes/items in a list
///
/// \param lref ListRef The list
/// \return int
///
int IntrusiveList_size(const IntrusiveListRef lref);

///
/// WARNING - THIS FUNCTION ALLOCATES MEMORY
///
///
// Add a new node to the back end of a list
///
/// \param lref ListRef A list
/// \param item void* An item to be added to the list
///
void IntrusiveList_add_back(IntrusiveListRef lref, void* item);

///
/// WARNING - THIS FUNCTION ALLOCATES MEMORY
///
///
// Add a new node to the front end of a list
///
/// \param lref ListRef A list
/// \param item void* An item to be added to the list
///
void IntrusiveList_add_front(IntrusiveListRef lref, void* item);

/// Gets the value of the void* item at the head of a list, without changing the
/// the list (does not remove the first node).
///
/// \param lref   ListRef a list Must not be NULL
/// \return void* The content/item in the first node on the list. NULL if the list is empty
///
void* IntrusiveList_first(const IntrusiveListRef lref);

/// Gets the value of the void* item in the node at the head of a list,
/// Returns the item void* in the node at the head of the list and frees
/// the node.
///
/// NOTE: removes the first node from the list..
///
/// \param lref   ListRef a list Must not be NULL
/// \return void* The content/item in the first node on the list. NULL if the list is empty
///
void* IntrusiveList_remove_first(IntrusiveListRef lref);

/// Gets the value of the void* item at the tail of a list, without changing the
/// the list (does not remove the first node).
///
/// \param lref   ListRef a list Must not be NULL
/// \return void* The content/item in the last node on the list. NULL if the list is empty
///
void* IntrusiveList_last(const IntrusiveListRef lref);

/// Gets the value of the void* item in the node at the the tail of a list,
/// returns the void* value and unlinks and frees the tail node
///
/// NOTE: removes the last node from the list..
///
/// \param lref   ListRef a list Must not be NULL
/// \return void* The content/item in the first node on the list. NULL if the list is empty
///
void* IntrusiveList_remove_last(IntrusiveListRef lref);

///
/// Gets a value of an opaque type that acts as an iterator for the list.
///
///
/// \param lref   ListRef a list Must not be NULL
/// \return ListIterator iterator pointing at the first/head of the list.
///         returns NULL if List is empty
///
IntrusiveListIterator IntrusiveList_iterator(const IntrusiveListRef lref);

///
/// Moves an iterator on to the next Node on the list.
/// returns NULL if goes off the end of the list
///
/// \param lref ListRef a List
/// \param itr  ListIterator a current iterator pointing at a node on the list, cannot NULL iterator
/// \return  THe next item on the list or NULL if there are no more nodes.
///
IntrusiveListIterator IntrusiveList_itr_next(const IntrusiveListRef lref, const IntrusiveListIterator itr);

///
/// Removes and frees the node pointed at by the itr, calls dealloc function on that nodes void* item field.
/// and sets to NULL the variable/argument holding the iterator.
///
/// \param lref ListRef
/// \param itr_adr  ListIterator* address of a list iterator variable holding a valid iterator.
///
void IntrusiveList_itr_remove(IntrusiveListRef lref, IntrusiveListIterator *itr_adr);

///
/// Gets the void* value of the item held in the Node pointed at by this iterator.
///
/// This is a dereference operation.
///
/// \param lref ListRef
/// \param itr  A valid iterator, cannot be NULL
/// \return     void* value held by node pointed at by itr
///
void* IntrusiveList_itr_unpack(IntrusiveListRef lref, IntrusiveListIterator itr);

IntrusiveListIterator List_find(IntrusiveListRef lref, void* needle);
/** @} */
#endif