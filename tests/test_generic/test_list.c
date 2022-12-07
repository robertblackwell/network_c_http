#include "mytype.h"
#include "mytype_table.h"

int main()
{
    struct MyTypeRef_vector *v = MyTypeRef_Table_new();
    MyTypeRef_Table_init(&v);

}