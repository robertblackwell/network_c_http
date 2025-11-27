#include <src/common/alloc.h>
#include <assert.h>
#include <stdlib.h>
void* allocator_alloc(Allocator* allocator, size_t size)
{
    return allocator->allocate(allocator, size);
}
void* allocator_realloc(Allocator* allocator, void* old_ptr, size_t size)
{
    return allocator->reallocate(allocator, old_ptr, size);
}
void allocator_dealloc(Allocator* allocator, void* ptr)
{
    if(allocator->deallocate) {
        allocator->deallocate(allocator, ptr);
    }
}
void allocator_reset(Allocator* allocator)
{
    if(allocator->reset) {
        allocator->reset(allocator);
    }
}
void allocator_destroy(Allocator* allocator)
{
    if(allocator->destroy) {
        allocator->destroy(allocator);
    }
}
