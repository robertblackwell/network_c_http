#ifndef T
#error you need to define T before including this header
#else
#define _CAT(A, B) A##_##B
#define CAT(A, B) _CAT(A, B)
#ifndef SUFFIX
#define SUFFIX T
#endif

#define G(N) CAT(N, SUFFIX)

typedef struct {
    T* data;
    size_t capacity;
    size_t length;
} G(Vector);

void G(vector_resize)(G(Vector)* v, size_t capacity);
void G(vector_append)(G(Vector)* v, T element);
void G(vector_clear)(G(Vector)* v);

#ifdef VECTOR_IMPLEMENTATION

void G(vector_resize)(G(Vector)* v, size_t capacity) {
    v->data = realloc(v->data, capacity * sizeof(T));
    v->capacity = capacity;
}

void G(vector_append)(G(Vector)* v, T element) {
    if (v->length >= v->capacity) {
        G(vector_resize)(v, v->capacity ? v->capacity * 2 : 8);
    }
    v->data[v->length] = element;
    v->length++;
}

void G(vector_clear)(G(Vector)* v) {
    free(v->data);
    v->data = NULL;
    v->capacity = 0;
    v->length = 0;
}

#endif

#undef G
#undef SUFFIX
#undef T
#endif