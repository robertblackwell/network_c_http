#ifndef mytype_wlist_h
#define mytype_list_h
#include "mytype.h"

#undef TYPE
#undef TYPED
#undef PREFIX
#define TYPE MyType
#define TYPED(THING) MyType##THING
#define PREFIX(THING) MyTypeWList##THING

#include "list_wrapper.h"


#endif