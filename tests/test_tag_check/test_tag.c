#include <string.h>
#include <stdlib.h>
#include <http_in_c/unittest.h>



#define TypeA_TAG "TYPEA"
#include <http_in_c/check_tag.h>

typedef struct TypeA_s {
    DECLARE_TAG;
    int another;
} TypeA, *TypeARef;

TypeARef TypeA_new()
{
    TypeARef this = malloc(sizeof(TypeA));
    SET_TAG(TypeA_TAG, this);
    return this;
}

#define TYPE TypeB
#define TypeB_TAG "TYPEB"
#include <http_in_c/check_tag.h>
#undef TYPE
#define CHECK_B(p) CHECK_TAG(TypeB_TAG, p)
#define FAIL_CHECK_B(p) FAIL_CHECK_TAG(TypeB_TAG, p)

typedef struct TypeB_s {
    DECLARE_TAG;
    int another;
} TypeB, *TypeBRef;
TypeBRef TypeB_new()
{
    TypeBRef this = malloc(sizeof(TypeB));
    SET_TAG(TypeB_TAG, this);
    return this;
}
int test_a()
{
    TypeARef a = TypeA_new();
    TypeBRef b = TypeB_new();
    CHECK_TAG(TypeA_TAG, a);
    CHECK_TAG(TypeB_TAG, b);
    FAIL_CHECK_TAG(TypeA_TAG, b)
    FAIL_CHECK_TAG(TypeB_TAG, a);

    return 0;
}

int main()
{
    UT_ADD(test_a);
    int rc = UT_RUN();
    return rc;
}