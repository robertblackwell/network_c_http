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
void  PREFIX(_add_back)(PREFIX(Ref) lref, TYPE* item);
void  PREFIX(_add_front)(PREFIX(Ref) lref, TYPE* item);
TYPE* PREFIX(_first)(const PREFIX(Ref) lref);
TYPE* PREFIX(_remove_first)(PREFIX(Ref) lref);
TYPE* PREFIX(_last)(const PREFIX(Ref) lref);
TYPE* PREFIX(_remove_last)(PREFIX(Ref) lref);
PREFIX(Iterator) PREFIX(_iterator)(const PREFIX(Ref) lref);
PREFIX(Iterator) PREFIX(_itr_next)(const PREFIX(Ref) lref, const PREFIX(Iterator) itr);
void PREFIX(_itr_remove) (PREFIX(Ref) lref, PREFIX(Iterator) *itr_adr);
TYPE* PREFIX(_itr_unpack) (PREFIX(Ref) lref, PREFIX(Iterator) itr);
PREFIX(Iterator) PREFIX(_find) (PREFIX(Ref) lref, TYPE* needle);
