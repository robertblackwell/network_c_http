#include <stdlib.h>
#include "mytype_table.h"

#define TYPE MyType
#define PREFIX MyType
#define TYPED(THING) MyType ## THING
#include "array_template.c"
#undef TYPE
#undef PREFIX


