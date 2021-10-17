#include <string.h>
#include <stdlib.h>
#include <c_http/unittest.h>



#define TYPE TypeA
#define TypeA_TAG "TYPEA"
#include <c_http/check_tag.h>
#undef TYPE
#define CHECK_A(p) CHECK_TAG(TypeA, p)
#define FAIL_CHECK_A(p) FAIL_CHECK_TAG(TypeA, p)

typedef struct TypeA_s {
    DECLARE_TAG(TypeA);
    int another;
} TypeA, *TypeARef;

TypeARef TypeA_new()
{
    TypeARef this = malloc(sizeof(TypeA));
    SET_TAG(TypeA, this);
    return this;
}

#define TYPE TypeB
#define TypeB_TAG "TYPEB"
#include <c_http/check_tag.h>
#undef TYPE
#define CHECK_B(p) CHECK_TAG(TypeB, p)
#define FAIL_CHECK_B(p) FAIL_CHECK_TAG(TypeB, p)

typedef struct TypeB_s {
    DECLARE_TAG(TypeB);
    int another;
} TypeB, *TypeBRef;
TypeBRef TypeB_new()
{
    TypeBRef this = malloc(sizeof(TypeB));
    SET_TAG(TypeB, this);
    return this;
}
int test_a()
{
    TypeARef a = TypeA_new();
    TypeBRef b = TypeB_new();
    CHECK_TAG(TypeA, a);
    CHECK_A(a);
    CHECK_B(b)
    FAIL_CHECK_B(a)
    FAIL_CHECK_TAG(TypeB, a);

    return 0;
}

int main()
{
    UT_ADD(test_a);
    int rc = UT_RUN();
    return rc;
}