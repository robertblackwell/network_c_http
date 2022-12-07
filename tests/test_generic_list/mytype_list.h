#ifndef mytype_list_h
#define mytype_list_h
#include "mytype.h"

#undef TYPE
#undef TYPED
#define TYPE MyType
#define TYPED(THING) MyType##THING
#include "list_template.h"


#endif