#include <common/alloc.h>
#include <assert.h>
#include <stdlib.h>
#ifdef __APPLE__
#include <malloc/malloc.h>
#endif

void* eg__alloc(size_t n)
{
    void* p = malloc(n);
    // #ifdef __APPLE__
    // int x = malloc_size(p);
    // #else
    // int x = malloc_usable_size(p);
    // #endif
    return p;
}
void eg__free(void* p)
{
    assert(p != NULL);
    free(p);
}