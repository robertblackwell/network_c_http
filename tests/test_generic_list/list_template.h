/**
 * This file and the related .c file provide a generic list of pointers to an (almost) arbitary
 * type.
 *
 * How to use list_template.h and list_template.c
 * ===============================================
 *
 * Say you have an aggregate type such as
 *  -   typedef struct MyType_s {....} MyType; *MyTypeRef;
 *
 * And you wish to make a list of such MyType things and you want
 * the type of that list to be called
 *  -   MySpecialChain and
 *  -   all the functions that operate n the list to be called things like
 *      MySpecialChain_new(), MySpecialChain_add_back(....)
 *
 * Then make two files:
 *  -   myspecialchain.h which consists of the following
 *
 *  #ifndef _myspecial_chain_h
 *  #define _myspecialchain_h
 *  under TYPE
 *  undef PREFIX
 *  #define TYPE MySpecialChain
 *  #define PREFIX(THING) MySpecialChain##THING
 *  #include list_template.h
 *  #endif
 *
 *  -   myspecialchaiin.c which consists of
 *  #include <.....what ever you need for MyType>
 *  #include <mytype.h>
 *  #include "myspecialchain.h"
 *  #include "list_template.c"
 *
 *  MyType requirements
 *  ===================
 *  The type MyType must provide (via mytype.h and mytype.c) two functions:
 *
 *  -   MyType_free(MyTYpe* this)
 *          which releases the memory held by an instance of MyType
 *  -   MyType_display(MyType* this)
 *          prints to stdout a formatted display of an instance of MyType
 */
// type of data to be held in the list
typedef void* ListItem;

struct TYPED(List_s);
typedef struct TYPED(List_s) TYPED(List), * TYPED(ListRef);

struct TYPED(ListNode_s);

typedef struct TYPED(ListNode_s) TYPED(ListNode);
typedef TYPED(ListNode) * TYPED(ListIterator), * TYPED(ListIter);
typedef void(*ListItemDeallocator)(void**);

TYPED(ListRef) TYPED(List_new) ();
void  TYPED(List_init)(TYPED(ListRef) lref);
void  TYPED(List_destroy)(TYPED(ListRef) lref);
void  TYPED(List_dispose)(TYPED(ListRef) *lref_adr);
void  TYPED(List_display)(const TYPED(ListRef) this);
int   TYPED(List_size)(const TYPED(ListRef) lref);
void  TYPED(List_add_back)(TYPED(ListRef) lref, void* item);
void  TYPED(List_add_front)(TYPED(ListRef) lref, void* item);
void* TYPED(List_first)(const TYPED(ListRef) lref);
void* TYPED(List_remove_first)(TYPED(ListRef) lref);
void* TYPED(List_last)(const TYPED(ListRef) lref);
void* TYPED(List_remove_last)(TYPED(ListRef) lref);
TYPED(ListIterator) TYPED(List_iterator)(const TYPED(ListRef) lref);
TYPED(ListIterator) TYPED(List_itr_next)(const TYPED(ListRef) lref, const TYPED(ListIterator) itr);
void TYPED(List_itr_remove) (TYPED(ListRef) lref, TYPED(ListIterator) *itr_adr);
void* TYPED(List_itr_unpack) (TYPED(ListRef) lref, TYPED(ListIterator) itr);
TYPED(ListIterator) TYPED(List_find) (TYPED(ListRef) lref, void* needle);
