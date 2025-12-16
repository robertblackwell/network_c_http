
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <rbl/unittest.h>
#include <allocators/malloc/alloc_malloc.h>
#include <src/common/iobuffer.h>
struct TestBlock
{
    size_t a;
    uint8_t mem[];
};
static void fill(void* p, char ch, size_t n)
{
    while(n-- > 0) {
        ((uint8_t*)p)[n] = ch;
    }
}
static void check_fill(void* p, char ch, size_t begin_n, size_t end_n)
{
    size_t n = begin_n;
    while(n < end_n) {
        char* q = &((char*)p)[n];
        assert(((uint8_t*)p)[n] == ch);
        n++;
    }
}

int test_simple()
{
    printf("XXXXXXXXXXXHello world from buffer test \n");
    Allocator* ma = malloc_allocator_create();
    void* p1 = allocator_alloc(ma, 120);
    allocator_dealloc(ma, p1);
    return 0;
}
int main()
{
    UT_ADD(test_simple);
    int rc = UT_RUN();
    return rc;
}