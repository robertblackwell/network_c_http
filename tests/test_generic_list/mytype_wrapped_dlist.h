#ifndef C_HTTP_MYTYPE_WRAPPED_DLIST_H
#define C_HTTP_MYTYPE_WRAPPED_DLIST_H
#include "mytype_A.h"

#undef TYPE
#undef TYPED
#undef PREFIX
#define TYPE MyTypeA
#define TYPED(THING) MyTypeA##THING
#define PREFIX(THING) MyTypeAWList##THING
#include <src/common/generics/dlist_wrapper_template.h>
//#include "dlist_wrapper_template.h"


#endif