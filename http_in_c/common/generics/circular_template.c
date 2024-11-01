
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <rbl/macros.h>
#include <rbl/check_tag.h>

#undef CIRCULAR_LIST_TAG_LENGTH
#define CIRCULAR_LIST_TAG_LENGTH TAG_LENGTH
#define CIRCULAR_TEMPLATE_TAG "TYPE##989"

#ifndef CIRCULAR_TEMPLATE_TAG
    #error CIRCULAR_TEMPLATE_TAG not defined
#endif
#ifndef RBL_SET_TAG
    #error SET_TAG_FIELD macro not found
#endif
#ifndef RBL_CHECK_TAG
    #error CHECK_TAG_FIELD macro not found
#endif

struct PREFIX(_s) {
    RBL_DECLARE_TAG
    int        capacity;
    int        head;
    int        tail_plus;
    TYPE       *list; // points to first element of an array of TYPED() objects
    RBL_DECLARE_END_TAG;
};

static void PREFIX(_set_tag)(PREFIX(Ref) this)
{
    RBL_SET_TAG(CIRCULAR_TEMPLATE_TAG, this)
    RBL_SET_END_TAG(CIRCULAR_TEMPLATE_TAG, this)
}
static void PREFIX(_check_tag)(PREFIX(Ref) this)
{
    RBL_SET_TAG(CIRCULAR_TEMPLATE_TAG, this)
    RBL_SET_END_TAG(CIRCULAR_TEMPLATE_TAG, this)
}
#undef CIRCULAR_LIST_TAG_LENGTH

// TODO - make these macros at some point
static TYPED(Ref) PREFIX(_entry_addr)(PREFIX(Ref) lstref, int index)
{
    return (&(lstref->list[index]));
}
static void PREFIX(_set_entry)(PREFIX(Ref) lstref, int index, TYPED() func)
{
    lstref->list[index] = func;
}
static TYPED() PREFIX(_get_entry)(PREFIX(Ref) lstref, int index)
{
    return (lstref->list[index]);
}
PREFIX(Ref) PREFIX(_new)(int capacity)
{
    PREFIX(Ref) st = malloc(sizeof(PREFIX()));
    PREFIX(_set_tag)(st);
    st->capacity = capacity;
    st->head = 0;
    st->tail_plus = 0;
    st->list = malloc(sizeof(TYPED()) * (capacity + 1));
    PREFIX(_check_tag)(st);
    return st;
}
void PREFIX(_free)(PREFIX(Ref) this)
{
    free(this->list);
    free(this);
}
void PREFIX(_add)(PREFIX(Ref) lstref, TYPED() func)
{
    PREFIX(_check_tag)(lstref);
    int tmp = (lstref->head + 1) % lstref->capacity;
    if(tmp == lstref->tail_plus) {
        RBL_ASSERT(false, "functor list is full cannot add another element");
    }
    lstref->head = tmp;
    PREFIX(_set_entry)(lstref, (lstref->head), func);
}
int PREFIX(_size)(PREFIX(Ref) lstref)
{
    PREFIX(_check_tag)(lstref);
    return (lstref->head + lstref->capacity - lstref->tail_plus) % lstref->capacity;
}
TYPED() PREFIX(_remove)(PREFIX(Ref) lstref)
{
    PREFIX(_check_tag)(lstref);
    if(PREFIX(_size)(lstref) == 0) {
        RBL_ASSERT(false, "cannot remove an element from an empty list");
    }
    int tmpix = (lstref->tail_plus + 1) % lstref->capacity;
    lstref->tail_plus = tmpix;

    return PREFIX(_get_entry)(lstref, tmpix);
}
void PREFIX(_display)(PREFIX(Ref) lstref)
{
    PREFIX(_check_tag)(lstref);
    int i = lstref->tail_plus + 1;
    while(true) {
        TYPE yy = (lstref->list[i]);
        printf("i: % d MyType.a %d \n", i, yy.a);
        if(i == lstref->head) {
            break;
        }
        i = (i + 1) % lstref->capacity;
    }
    TYPE x1 = *(lstref->list + 0);
    TYPE x2 = *(lstref->list + 1);
    TYPE x3 = *(lstref->list + 2);
    printf("circular list\n");
}