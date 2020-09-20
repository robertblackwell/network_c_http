#ifndef c_eg_alloc_h
#define c_eg_alloc_h
#include <stddef.h>
/*
 * Use the macros not the functions
 */
void* eg__alloc(size_t n);
void eg__free(void* p);

#define eg_alloc(n) eg__alloc(n)
#define eg_free(p) eg__free(p)

#endif