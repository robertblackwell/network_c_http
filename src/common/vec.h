#ifndef c_http_common_vec_h
#define c_http_common_vec_h
#include <stddef.h>
#define _CAT(A,B) A##_##B
#define CAT(A,B) _CAT(A,B)
typedef struct Vec_s Vec_t, *Vec_p;

Vec_p vec_new(size_t initial_capacity);
void* vec_at(Vec_p v, size_t index);
void vec_append(Vec_p v, void* data);
size_t vec_size(Vec_p v);
void vec_free(Vec_p v);

#endif