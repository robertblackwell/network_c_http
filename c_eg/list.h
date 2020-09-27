#ifndef c_ceg_list_h
#define c_ceg_list_h

// type of data to be held in the list
typedef void* ListItem;

///
/// List is an opaque type. Can hold anonamous objects of type void*
/// Typically used as the base for lists of specific types by casting
/// MyType*/MyTypeRef to void*.
/// But can also hold int, long, char etc
///
struct List_s;
typedef struct List_s List;

//Internal - the type of nodes on the chain of nodes managed by
// a List type.
struct ListNode_s;

typedef struct ListNode_s ListNode;
typedef ListNode* ListIterator;
///
/// type definition for a function that knows how to deallocate
/// the void* item field in a ListNode.
///
/// By convention such functions take a void** rather than a void*
/// so that the the ListNode field can be set to NULL after deallocation.
///
typedef void(*ListItemDeallocator)(void**);

///
/// WARNING - THIS FUNCTION ALLOCATES MEMORY
///
///
/// Dynamically allocates a new List and return a List*, reference to the opaque List structure.
///
/// \param dealloc ListItemDeallocator
/// \return List*
///
List* List_new(ListItemDeallocator dealloc);

///
/// Initializes a list structure for which the memory has already been acquired.
/// Typically for local variables of type List, called as:
///
///     List mylist;
///     List_init(&mylist, mydealloc_function);
///
/// \param lref    List* or List*
/// \param dealloc ListItemDeallocator
///
void List_init(List* lref, ListItemDeallocator dealloc);

///
/// De-init an existing List instance
/// In particular this empties the list and if appropriate deallocates
/// all the nodes of the list and applies dealloc function to each list nodes
/// void* content.
/// But does not deallocate the memory for the list structure itself.
///
void List_destroy(List* lref);

///
/// Frees all memory associated with the list.
///
/// NOTE: the List** lref_ptr argument maybe a little unusual. This function is used as follows:
///
///   List* mylist = List_new(mydeallocator_function);
///
///   ..... do stuff with the list
///
///   List_free(&mylist); Note: the & on the List*
///                       Note also after the return from List_free() mylist == NULL
///
///
void List_free(List** lref_ptr);

///
/// Prints a dump of the list to stdout
///
/// \param this List* Note the name
///
void List_display(List* this);

///
/// Returns number of nodes/items in a list
///
/// \param lref List* The list
/// \return int
///
int List_size(List* lref);

///
/// WARNING - THIS FUNCTION ALLOCATES MEMORY
///
///
// Add a new node to the back end of a list
///
/// \param lref List* A list
/// \param item void* An item to be added to the list
///
void List_add_back(List* lref, void* item);

///
/// WARNING - THIS FUNCTION ALLOCATES MEMORY
///
///
// Add a new node to the front end of a list
///
/// \param lref List* A list
/// \param item void* An item to be added to the list
///
void List_add_front(List* lref, void* item);

/// Gets the value of the void* item at the head of a list, without changing the
/// the list (does not remove the first node).
///
/// \param lref   List* a list Must not be NULL
/// \return void* The content/item in the first node on the list. NULL if the list is empty
///
void* List_first(List* lref);

/// Gets the value of the void* item in the node at the head of a list,
/// Returns the item void* in the node at the head of the list and frees
/// the node.
///
/// NOTE: removes the first node from the list..
///
/// \param lref   List* a list Must not be NULL
/// \return void* The content/item in the first node on the list. NULL if the list is empty
///
void* List_remove_first(List* lref);

/// Gets the value of the void* item at the tail of a list, without changing the
/// the list (does not remove the first node).
///
/// \param lref   List* a list Must not be NULL
/// \return void* The content/item in the last node on the list. NULL if the list is empty
///
void* List_last(List* lref);

/// Gets the value of the void* item in the node at the the tail of a list,
/// returns the void* value and unlinks and frees the tail node
///
/// NOTE: removes the last node from the list..
///
/// \param lref   List* a list Must not be NULL
/// \return void* The content/item in the first node on the list. NULL if the list is empty
///
void* List_remove_last(List* lref);

///
/// Gets a value of an opaque type that acts as an iterator for the list.
///
///
/// \param lref   List* a list Must not be NULL
/// \return ListIterator iterator pointing at the first/head of the list.
///         returns NULL if List is empty
///
ListIterator List_iterator(List* lref);

///
/// Moves an iterator on to the next Node on the list.
/// returns NULL if goes off the end of the list
///
/// \param lref List* a List
/// \param itr  ListIterator a current iterator pointing at a node on the list, cannot NULL iterator
/// \return  THe next item on the list or NULL if there are no more nodes.
///
ListIterator List_itr_next(List* lref, ListIterator itr);

///
/// Removes and frees the node pointed at by the itr, calls dealloc function on that nodes void* item field.
/// and sets to NULL the variable/argument holding the iterator.
///
/// \param lref List*
/// \param itr  ListIterator a valid iterator for lref.
///
void List_itr_remove(List* lref, ListIterator* itr);

///
/// Gets the void* value of the item held in the Node pointed at by this iterator.
///
/// This is a dereference operation.
///
/// \param lref List*
/// \param itr  A valid iterator, cannot be NULL
/// \return     void* value held by node pointed at by itr
///
void* List_itr_unpack(List* lref, ListIterator itr);

#endif