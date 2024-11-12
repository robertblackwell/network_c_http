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
 *  -   all the functions that operate on the list to be called things like
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

struct PREFIX(_s);
typedef struct PREFIX(_s) PREFIX(), * PREFIX(Ref);

struct PREFIX(Node_s);

typedef struct PREFIX(Node_s) PREFIX(Node);
typedef PREFIX(Node) * PREFIX(Iterator), * PREFIX(Iter);
typedef void(*ListItemDeallocator)(void**);

PREFIX(Ref) PREFIX(_new) ();
void  PREFIX(_init)(PREFIX(Ref) lref);
void  PREFIX(_destroy)(PREFIX(Ref) lref);
void  PREFIX(_dispose)(PREFIX(Ref) *lref_adr);
void  PREFIX(_display)(const PREFIX(Ref) this);
int   PREFIX(_size)(const PREFIX(Ref) lref);
/**
 * add_back and add_front could be passed a TYPE by reference, in that case the following
 * two function signatures would be
void  PREFIX(_add_back)(PREFIX(Ref) lref, TYPE* item);
void  PREFIX(_add_front)(PREFIX(Ref) lref, TYPE* item);
 * But since we are storing the value of the item in the list the contents would still
 * be copied into the list enry.
 * To make this explicit the two following signatures are TYPE item.
 * We expect that a good optimising compiler will eliminate cone of the copies.
 */
void  PREFIX(_add_back)(PREFIX(Ref) lref, TYPE item);
void  PREFIX(_add_front)(PREFIX(Ref) lref, TYPE item);
/**
 * The next two functions return a list item by reference,
 * These calls do not remove the entry from the list so
 * there is no problem about dangling pointers through these functions
 */
TYPE* PREFIX(_first)(const PREFIX(Ref) lref);
TYPE* PREFIX(_last)(const PREFIX(Ref) lref);
/**
 * The njext two functions actually remove the target item, hence the value of the
 * item is returned to the caller; not a reference..
 */
TYPE PREFIX(_remove_first)(PREFIX(Ref) lref);
TYPE PREFIX(_remove_last)(PREFIX(Ref) lref);
PREFIX(Iterator) PREFIX(_iterator)(const PREFIX(Ref) lref);
PREFIX(Iterator) PREFIX(_itr_next)(const PREFIX(Ref) lref, const PREFIX(Iterator) itr);

void PREFIX(_itr_remove) (PREFIX(Ref) lref, PREFIX(Iterator) *itr_adr);
TYPE PREFIX(_itr_unpack) (PREFIX(Ref) lref, PREFIX(Iterator) itr);

/**
 * This function will search the list for an instance of TYPE that equals needle.
 * This function will attempt to call a function:
 *
 *      bool TYPED(_are_equal)(TYPE a, TYPE b)
 *
 * Which should check for equality of value. Unlike the void* list where we
 * checked equality of reference.
 */
PREFIX(Iterator) PREFIX(_find) (PREFIX(Ref) lref, TYPE needle);
