#ifndef mytype_circular_h
#define mytype_circular_h
#include "mytype_A.h"

#undef TYPE
#undef TYPED
#undef PREFIX
#define TYPE MyTypeA
#define TYPED(THING) MyTypeA##THING
#define PREFIX(THING) MyTypeA_Circular##THING
#include "circular_template.h"


#endif