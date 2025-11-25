#include <src/common/vec.h>
#include <rbl/check_tag.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
struct Vec_s
{
    RBL_DECLARE_TAG;
    size_t capacity;
    size_t size;
    void** data;
    RBL_DECLARE_END_TAG;
};
#define VEC_Tag "VECTAG"
Vec_p vec_new(size_t initial_capacity)
{
    void* mem = malloc(sizeof(Vec_t));
    assert(mem);
    Vec_p v = mem;
    RBL_SET_TAG(VEC_Tag, v)
    RBL_SET_END_TAG(VEC_Tag, v)
    v->capacity = initial_capacity;
    v->size = 0;
    v->data = malloc(initial_capacity * sizeof(void*));
    return v;
}
void* vec_at(Vec_p v, size_t index)
{
    RBL_CHECK_TAG(VEC_Tag, v)
    RBL_CHECK_END_TAG(VEC_Tag, v)
    assert((index >= 0) && (index < v->size));
    void* a = (v->data[index]);
    return a;
}
void vec_append(Vec_p v, void* data)
{
    RBL_CHECK_TAG(VEC_Tag, v)
    RBL_CHECK_END_TAG(VEC_Tag, v)
    if (v->size == v->capacity) {
        void* new_data = realloc(v->data, v->capacity * 2 * sizeof(void*));
        assert(new_data);
        v->capacity *= 2;
        void* old_data = v->data;
        v->data = new_data;
        // memcpy(new_data, old_data, v->size * sizeof(void*));
    }
    v->data[v->size++] = data;
}
size_t vec_size(Vec_p v)
{
    RBL_CHECK_TAG(VEC_Tag, v)
    RBL_CHECK_END_TAG(VEC_Tag, v)
    return v->size;
}
void vec_free(Vec_p v)
{
    RBL_CHECK_TAG(VEC_Tag, v)
    RBL_CHECK_END_TAG(VEC_Tag, v)
    free(v->data);
    free(v);
}
