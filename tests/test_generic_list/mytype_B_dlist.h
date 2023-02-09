#ifndef C_HTTP_MYTYPE_B_DLIST_H
#define C_HTTP_MYTYPE_B_DLIST_H
#include "mytype_B.h"

#undef TYPE
#undef TYPED
#undef PREFIX
#define TYPE MyTypeB
#define TYPED(THING) MyTypeB##THING
#define PREFIX(THING) MyTypeBChain##THING
#include <http_in_c/common/generics/dlist_template.h>
//#include "dlist_template.h"


#endif