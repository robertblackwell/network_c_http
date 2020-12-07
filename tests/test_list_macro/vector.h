#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define vector_MIN_CAP 32

#define vector_def_struct(T) typedef struct T##_vector T##_vector, * T##_vectorRef;

#define vector_def_new(T)    T##_vectorRef T##_vector_new();
#define vector_def_init(T)  void T##_vector_init(T##_vector *vec);

#define vector_def_get(T) void *T##_vector_get(T##_vector *vec, size_t idx);
#define vector_def_set(T) void T##_vector_set(T##_vector *vec, size_t idx, T data);

#define vector_def_push(T) void T##_vector_push(T##_vector *vec, T data);

#define vector_def(T)                                                              \
  vector_def_struct(T);                                                            \
  vector_def_new(T) vector_def_init(T) vector_def_get(T) vector_def_set(T) vector_def_push(T)
