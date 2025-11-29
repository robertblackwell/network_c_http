#include <src/common/alloc.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define MALLOC_ALLO "MALLOC"
void* malloc_allocate(Allocator* allocator, size_t size)
{
    void* tmp = malloc(size);
    return tmp;
}
void* malloc_reallocate(Allocator* allocator, void* ptr, size_t size)
{
    void* tmp = realloc(ptr, size);
    return tmp;
}
void  malloc_deallocate(Allocator* allocator, void* ptr)
{
    free(ptr);
}
void malloc_reset(Allocator* allocator)
{

}
void malloc_destroy(Allocator* allocator)
{
    free(allocator);
}
Allocator* malloc_allocator_create()
{
    Allocator* allocator = malloc(sizeof(Allocator));
    RBL_SET_TAG(MALLOC_ALLO, allocator)
    RBL_SET_END_TAG(MALLOC_ALLO, allocator)
    allocator->allocate = malloc_allocate;
    allocator->reallocate = &malloc_reallocate;
    allocator->deallocate = &malloc_deallocate;
    allocator->reset = NULL;
    allocator->destroy = &malloc_destroy;
    return allocator;
}