#ifndef mytype_table_h
#define mytype_table_h
#include "mytype.h"

#define TYPE MyType
#define TYPED(THING) MyType ## THING
#define PREFIX MyType

#include "array_template.h"
#undef TYPE
#undef PREFIX
#undef TYPED
#endif