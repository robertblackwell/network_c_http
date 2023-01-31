#ifndef _dlinked_list_h
#define _dlinked_list_h

/**
 * @addtogroup group_list
 * @{
*/

// type of data to be held in the list
typedef void* DListItem;

///
/// List is an opaque type. Can hold anonamous objects of type void*
/// Typically used as the base for lists of specific types by casting
/// MyType*/MyTypeRef to void*.
/// But can also hold int, long, char etc
///
struct DList_s;
typedef struct DList_s DList, *DListRef;

//Internal - the type of nodes on the chain of nodes managed by
// a List type.
struct DListNode_s;

typedef struct DListNode_s DListNode;
typedef DListNode* DListIterator, *DListIter;
///
/// type definition for a function that knows how to deallocate
/// the void* item field in a DListNode.
///
/// By convention such functions take a void** rather than a void*
/// so that the the DListNode field can be set to NULL after deallocation.
///
typedef void(*DListItemDeallocator)(void**);

///
/// WARNING - THIS FUNCTION ALLOCATES MEMORY
///
///
/// Dynamically allocates a new DList and return a DListRef, reference to the opaque DList structure.
///
/// \param dealloc DListItemDeallocator
/// \return DListRef
///
DListRef DList_new(DListItemDeallocator dealloc);

///
/// Initializes a list structure for which the memory has already been acquired.
/// Typically for local variables of type DList, called as:
///
///     DList mylist;
///     DList_init(&mylist, mydealloc_function);
///
/// \param lref    DListRef or DListRef
/// \param dealloc DListItemDeallocator
///
void DList_init(DListRef lref, DListItemDeallocator dealloc);

///
/// De-init an existing DList instance
/// In particular this empties the list and if appropriate deallocates
/// all the nodes of the list and applies dealloc function to each list nodes
/// void* content.
/// But does not deallocate the memory for the list structure itself.
///
void DList_destroy(DListRef lref);

///
/// Frees all memory associated with the list.
///
/// NOTE: the DListRef* lref_ptr argument maybe a little unusual. This function is used as follows:
///
///   DListRef mylist = DList_new(mydeallocator_function);
///
///   ..... do stuff with the list
///
///   DList_dispose(&mylist); Note: the & on the DListRef
///                       Note also after the return from DList_dispose() mylist == NULL
///
///
void DList_dispose(DListRef *lref_adr);

///
/// Prints a dump of the list to stdout
///
/// \param this DListRef Note the name
///
void DList_display(const DListRef this);

///
/// Returns number of nodes/items in a list
///
/// \param lref DListRef The list
/// \return int
///
int DList_size(const DListRef lref);

///
/// WARNING - THIS FUNCTION ALLOCATES MEMORY
///
///
// Add a new node to the back end of a list
///
/// \param lref DListRef A list
/// \param item void* An item to be added to the list
///
void DList_add_back(DListRef lref, void* item);

///
/// WARNING - THIS FUNCTION ALLOCATES MEMORY
///
///
// Add a new node to the front end of a list
///
/// \param lref DListRef A list
/// \param item void* An item to be added to the list
///
void DList_add_front(DListRef lref, void* item);

/// Gets the value of the void* item at the head of a list, without changing the
/// the list (does not remove the first node).
///
/// \param lref   DListRef a list Must not be NULL
/// \return void* The content/item in the first node on the list. NULL if the list is empty
///
void* DList_first(const DListRef lref);

/// Gets the value of the void* item in the node at the head of a list,
/// Returns the item void* in the node at the head of the list and frees
/// the node.
///
/// NOTE: removes the first node from the list..
///
/// \param lref   DListRef a list Must not be NULL
/// \return void* The content/item in the first node on the list. NULL if the list is empty
///
void* DList_remove_first(DListRef lref);

/// Gets the value of the void* item at the tail of a list, without changing the
/// the list (does not remove the first node).
///
/// \param lref   DListRef a list Must not be NULL
/// \return void* The content/item in the last node on the list. NULL if the list is empty
///
void* DList_last(const DListRef lref);

/// Gets the value of the void* item in the node at the the tail of a list,
/// returns the void* value and unlinks and frees the tail node
///
/// NOTE: removes the last node from the list..
///
/// \param lref   DListRef a list Must not be NULL
/// \return void* The content/item in the first node on the list. NULL if the list is empty
///
void* DList_remove_last(DListRef lref);

///
/// Gets a value of an opaque type that acts as an iterator for the list.
///
///
/// \param lref   DListRef a list Must not be NULL
/// \return DListIterator iterator pointing at the first/head of the list.
///         returns NULL if DList is empty
///
DListIterator DList_iterator(const DListRef lref);

///
/// Moves an iterator on to the next Node on the list.
/// returns NULL if goes off the end of the list
///
/// \param lref DListRef a DList
/// \param itr  DListIterator a current iterator pointing at a node on the list, cannot NULL iterator
/// \return  THe next item on the list or NULL if there are no more nodes.
///
DListIterator DList_itr_next(const DListRef lref, const DListIterator itr);

///
/// Removes and frees the node pointed at by the itr, calls dealloc function on that nodes void* item field.
/// and sets to NULL the variable/argument holding the iterator.
///
/// \param lref DListRef
/// \param itr_adr  DListIterator* address of a list iterator variable holding a valid iterator.
///
void DList_itr_remove(DListRef lref, DListIterator *itr_adr);

///
/// Gets the void* value of the item held in the Node pointed at by this iterator.
///
/// This is a dereference operation.
///
/// \param lref DListRef
/// \param itr  A valid iterator, cannot be NULL
/// \return     void* value held by node pointed at by itr
///
void* DList_itr_unpack(DListRef lref, DListIterator itr);

DListIterator DList_find(DListRef lref, void* needle);
/** @} */
#endif