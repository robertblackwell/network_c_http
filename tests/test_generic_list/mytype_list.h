#ifndef mytype_list_h
#define mytype_list_h
#include "mytype.h"

#undef TYPE
#undef TYPED
#undef PREFIX
#define TYPE MyType
#define PREFIX(THING) MyType##THING
#include "list_template.h"


#endif