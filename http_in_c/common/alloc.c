#include <http_in_c/common/alloc.h>
#include <assert.h>
#include <malloc.h>

void* eg__alloc(size_t n)
{
    void* p = malloc(n);
    int x = malloc_usable_size(p);
    return p;
}
void eg__free(void* p)
{
    assert(p != NULL);
    free(p);
}