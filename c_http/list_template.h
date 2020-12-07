//// LIST_ITEM_TYPE
//
//#define TNode         T ## _Node
//#define TNode_s       T ## _Node_s
//#define TNodeRef      T ## _NodeRef
//
//#define TListItem     T ## _Item
//#define TListItemRef  * TListItem
//
//#define TRef         T ## Ref
//#define TList_s
//#define TList        T ## _List
//#define TListRef     T ## _ListRef
//
//#define TListIter    T ## _ListIter
//#define LT TList
//

#define ListInterface(T)                                                \
typedef struct T ## List_s  T ## List, * TListRef;                      \
typedef struct T ## ListNode_s T ## ListNode, * T ## ListNodeRef;       \
typedef T ## ListNodeRef T ## ListIter;                                 \
typedef void(*ListItemDeallocator)(void**);                             \
                                                                        \
                                                                        \
T ## ListRef    T ## List ## _new(ListItemDeallocator dealloc);                     \
void            T ## List ## _init(T ## ListRef lref, ListItemDeallocator dealloc);      \
void            T ## List ## _destroy(T ## ListRef lref);                                \
void            T ## List ## _free(T ## ListRef *lref_adr);                              \
void            T ## List ## _display(const T ## ListRef this);                          \
int             T ## List ## _size(const T ## ListRef lref);                             \
void            T ## List ## _add_back(T ## ListRef lref, void* item);                   \
void            T ## List ## _add_front(T ## ListRef lref, void* item);                  \
T ## Ref        T ## List ## _first(const T ## ListRef lref);                            \
T ## Ref        T ## List ## _remove_first(T ## ListRef lref);                           \
T ## Ref        T ## List ## _last(const T ## ListRef lref);                             \
T ## Ref        T ## List ## _remove_last(T ## ListRef lref);                            \
T ## ListIter   T ## List ## _iterator(const T ## ListRef lref);                         \
T ## ListIter   T ## List ## _itr_next(const T ## ListRef lref, const ListIterator itr); \
void            T ## List ## _itr_remove(T ## ListRef lref, ListIterator *itr_adr);      \
T ## ListItem   T ## List ## _itr_unpack(T ## ListRef lref, ListIterator itr);

#undef LISTNODE
#undef LISTITEM
#undef LISTTYPE