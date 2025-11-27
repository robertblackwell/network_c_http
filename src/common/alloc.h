#ifndef c_http_alloc_h
#define c_http_alloc_h
#include <stddef.h>
#include <rbl/check_tag.h>
typedef struct Allocator_s Allocator;

/**
 * All instances of an allocator must provide a struct type with the corresponding
 * 5 (five) function pointers at the beginning of the struct
 */
struct Allocator_s
{
    RBL_DECLARE_TAG;
    void*(*allocate)(Allocator* allocator, size_t size);
    void*(*reallocate)(Allocator* allocator, void* ptr, size_t size);
    void(*deallocate)(Allocator* allocator, void* p);
    void(*reset)(Allocator* allocator);
    void(*destroy)(Allocator* allocator);
    RBL_DECLARE_END_TAG
};

void* allocator_alloc(Allocator* allcator, size_t size);
void* allocator_realloc(Allocator* allocator, void* old_ptr, size_t size);
void allocator_dealloc(Allocator* allocator, void* ptr);
void allocator_reset(Allocator* allocator);
void allocator_destroy(Allocator* allocator);
/** @} */
#endif