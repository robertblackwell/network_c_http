#include <string.h>
#include <stdlib.h>
#include <rbl/unittest.h>


#define RBL_TAG_CHECK_ON

#define TypeA_TAG "TYPEA"
#include <rbl/check_tag.h>

typedef struct TypeA_s {
    RBL_DECLARE_TAG;
    int another;
} TypeA, *TypeARef;

TypeARef TypeA_new()
{
    TypeARef this = malloc(sizeof(TypeA));
    RBL_SET_TAG(TypeA_TAG, this);
    return this;
}

#define TYPE TypeB
#define TypeB_TAG "TYPEB"
#include <rbl/check_tag.h>
#undef TYPE
#define CHECK_B(p) RBL_CHECK_TAG(TypeB_TAG, p)
#define FAIL_CHECK_B(p) FAIL_CHECK_TAG(TypeB_TAG, p)

typedef struct TypeB_s {
    RBL_DECLARE_TAG;
    int another;
    RBL_DECLARE_END_TAG;
} TypeB, *TypeBRef;

TypeBRef TypeB_new()
{
    TypeBRef this = malloc(sizeof(TypeB));
    RBL_SET_TAG(TypeB_TAG, this);
    RBL_SET_END_TAG(TypeB_TAG, this)
    return this;
}
int test_a()
{
    TypeARef a = TypeA_new();
    TypeBRef b = TypeB_new();
    RBL_CHECK_TAG(TypeA_TAG, a);
//    following lines will not compile
//    bool xx = RBL_END_TAG_VALID(TypeA_TAG, a);
//    UT_TRUE((RBL_END_TAG_VALID(TypeA_TAG, a) == false));
    RBL_CHECK_TAG(TypeB_TAG, b);
    RBL_CHECK_END_TAG(TypeB_TAG, b)
    RBL_FAIL_CHECK_TAG(TypeA_TAG, b)
    RBL_FAIL_CHECK_TAG(TypeB_TAG, a);

    RBL_INVALIDATE_END_TAG(b)
    bool xx2 = RBL_END_TAG_VALID(TypeB_TAG, b);
    UT_TRUE((!xx2))
    {
        RBL_INVALIDATE_STRUCT(b, TypeB);
        bool xx1 = RBL_TAG_VALID(TypeB_TAG, b);
        UT_TRUE((!xx1));
        bool xx2 = RBL_END_TAG_VALID(TypeB_TAG, b);
        UT_TRUE((!xx2))
    }
    return 0;
}

int main()
{
    UT_ADD(test_a);
    int rc = UT_RUN();
    return rc;
}