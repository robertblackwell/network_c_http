#ifndef mytype_a_wlist_h
#define mytype_a_list_h
#include "mytype_A.h"

#undef TYPE
#undef TYPED
#undef PREFIX
#define TYPE MyTypeA
#define TYPED(THING) MyTypeA##THING
#define PREFIX(THING) MyTypeAWList##THING

#include "dlist_wrapper.h"


#endif