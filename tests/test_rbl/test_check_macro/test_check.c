#include <stdio.h>
#include <stdlib.h>

#define CHECK(A, label) do { \
    if((A) == NULL) goto label; \
} while(0);

#define CHECK2(A, B, label) do { \
    A = NULL; \
    A = B; \
    if(A == NULL) goto label; \
} while(0);


void* eg_alloc(int n)
{
    return NULL;
}

struct TestData_s {
    int data[1000];
} TestData, *TestDataRef;

int main()
{

    void* p;
    CHECK2(p, eg_alloc(sizeof(TestData)), cleanup);

    CHECK(p = eg_alloc(sizeof(TestData)), cleanup);
    return 0;
    cleanup:
        printf("Got to clean up\n");
        return 1;
}