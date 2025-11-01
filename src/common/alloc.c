#include <src/common/alloc.h>
#include <assert.h>
#include <stdlib.h>
#ifdef LINUX_FLAG
#include <malloc.h>
#endif
void* eg__alloc(size_t n)
{
    void* p = malloc(n);
    #ifdef LINUX_FLAG
    int x = malloc_usable_size(p);
    #endif
    return p;
}
void eg__free(void* p)
{
    assert(p != NULL);
    free(p);
}