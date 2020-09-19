#include <c_eg/alloc.h>
#include <stdlib.h>
#include <assert.h>

void* eg__alloc(size_t n)
{
    return malloc(n);
}
void eg__free(void* p)
{
    assert(p != NULL);
    free(p);
}