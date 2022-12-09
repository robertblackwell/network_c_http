#ifndef mytype_b_dlist_h
#define mytype_b_dlist_h
#include "mytype_B.h"

#undef TYPE
#undef TYPED
#undef PREFIX
#define TYPE MyTypeB
#define TYPED(THING) MyTypeB##THING
#define PREFIX(THING) MyTypeBChain##THING
#include "dlist_template.h"


#endif