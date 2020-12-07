#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define vector_MIN_CAP 32

#define vector_struct(T)                                                       \
  struct T##_vector {                                                  \
    T *buf;                                                                    \
    size_t capacity;                                                           \
    size_t size;                                                               \
  } ;

#define vector_init(T)                                                         \
  void T##_vector_init(T##_vector *vec) {                                      \
    vec->capacity = vector_MIN_CAP;                                            \
    vec->buf = malloc(sizeof(T) * vec->capacity);                              \
    vec->size = 0;                                                             \
  }

#define vector_new(T)                                                          \
  T##_vectorRef T##_vector_new() {                                              \
    T##_vector* vec = malloc(sizeof(T##_vector));                               \
    vec->capacity = vector_MIN_CAP;                                            \
    vec->buf = malloc(sizeof(T) * vec->capacity);                              \
    vec->size = 0;                                                             \
  }


#define vector_get(T)                                                          \
  void *T##_vector_get(T##_vector *vec, size_t idx) { return vec->buf + idx; }

#define vector_set(T)                                                          \
  void T##_vector_set(T##_vector *vec, size_t idx, T data) {                   \
    vec->buf[idx] = data;                                                      \
  }

#define vector_push(T)                                                         \
  void T##_vector_push(T##_vector *vec, T data) {                              \
    if (vec->size == vec->capacity) {                                          \
      vec->capacity *= 2;                                                      \
      vec->buf = realloc(vec->buf, sizeof(T) * vec->capacity);                 \
    }                                                                          \
    T##_vector_set(vec, vec->size++, data);                                    \
  }

#define vector(T)                                                              \
  vector_struct(T);                                                            \
  vector_new(T) vector_init(T) vector_get(T) vector_set(T) vector_push(T)
