#ifndef c_http_alloc_h
#define c_http_alloc_h
#include <stddef.h>

/**
 * @addtogroup group_alloc
 * @{
 */
/// used to mark return type of functions that allocate memory
#define MEMALLOC(type) type
#define IFNULL(A, label) do { \
    if((A) == NULL) goto label; \
} while(0);
/*
 * Use the macros not the functions
 */
void* eg__alloc(size_t n);
void eg__free(void* p);

#define eg_alloc(n) eg__alloc(n)
#define eg_free(p) eg__free(p)

/** @} */
#endif