#ifndef C_HTTP_MYTYPE_CIRCULAR_LIST_H
#define C_HTTP_MYTYPE_CIRCULAR_LIST_H
#include "mytype_A.h"

#undef TYPE
#undef TYPED
#undef PREFIX
#define TYPE MyTypeA
#define TYPED(THING) MyTypeA##THING
#define PREFIX(THING) MyTypeA_Circular##THING
#include <c_http//common//generics/circular_template.h>
//#include "circular_template.h"


#endif