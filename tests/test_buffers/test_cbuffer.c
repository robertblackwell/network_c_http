
#include <stdio.h>
#include <string.h>
#include <rbl/unittest.h>
#include <src/common/cbuffer.h>
#include <src/common/buffer_chain.h>
#include <src/common/iobuffer.h>
#include <common/alloc_malloc.h>
#include <common/arena.h>

//#define IOB_FILL
#ifdef IOB_FILL
#define IOB_TERM_CHAR '?';
#define IOB_FILL_CHAR '+'
#else
#define IOB_TERM_CHAR (char)0x00;
#define IOB_FILL_CHAR '+'
#endif
char* cstr_concat(char* s1, char* s2)
{
    size_t n1 = strlen(s1);
    size_t n2 = strlen(s2);
    size_t sz = strlen(s1) + strlen(s2);
    char* result;
    asprintf(&result, "%s%s", s1, s2);
    return result;
}

int test_simple()
{
    printf("XXXXXXXXXXXHello world from buffer test \n");
    UT_EQUAL_INT(1, 1);
    return 0;
}
int test_make_buffer()
{
#ifdef CBUF_ALLOCATOR_MALLOC
    Allocator* ma = malloc_allocator_create();
#else
    Allocator* ma = arena_allocator_create(4*1024);
#endif

    CbufferRef b = Cbuffer_new_with_allocator(ma);
    printf("m_size %ld \n", Cbuffer_size(b));
    printf("m_capacity %ld \n", Cbuffer_capacity(b));
    printf("m_cptr %lx \n", (long)Cbuffer_data(b));
    UT_NOT_EQUAL_PTR((b), NULL);
    UT_EQUAL_INT(Cbuffer_size(b), 0);
    UT_NOT_EQUAL_PTR((void*)Cbuffer_data(b), NULL);
    Cbuffer_free(b); b = NULL;
    UT_EQUAL_PTR(b, NULL);
    allocator_destroy(ma);
    return 0;
}
int test_expansion()
{
#ifdef CBUF_ALLOCATOR_MALLOC
    Allocator* ma = malloc_allocator_create();
#else
    Allocator* ma = arena_allocator_create(4*1024);
#endif
    char* s1 = cstr_concat("","");
    char* extra = "abcedfghijklmnopqrstuvwxyz01923456789";
    CbufferRef b2 = Cbuffer_new_with_allocator(ma);
    for(int i = 0; i < 5; i++) {
        Cbuffer_append(b2, (void*)extra, strlen(extra));
    }

    printf("b2 m_size %ld \n", Cbuffer_size(b2));
    printf("b2 m_capacity %ld \n", Cbuffer_capacity(b2));
    printf("b2 m_cptr %lx \n", (long)Cbuffer_data(b2));
    UT_EQUAL_INT(5*strlen(extra), Cbuffer_size(b2));
    Cbuffer_free(b2); b2 = NULL;
    free(s1);
    UT_EQUAL_PTR(b2, NULL);
    allocator_destroy(ma);
    return 0;
}
int test_expand()
{
#ifdef CBUF_ALLOCATOR_MALLOC
    Allocator* ma = malloc_allocator_create();
#else
    Allocator* ma = arena_allocator_create(4*1024);
#endif
    char* extra = "abcedfghijklmnopqrstuvwxyz01923456789";
    CbufferRef b2 = Cbuffer_from_cstring(extra, ma);
    size_t sz = Cbuffer_size(b2);
    size_t cp = Cbuffer_capacity(b2);
    Cbuffer_expand(b2, cp*3);
    allocator_destroy(ma);
    return 0;
}
int test_big_expansion()
{
#ifdef CBUF_ALLOCATOR_MALLOC
    Allocator* ma = malloc_allocator_create();
#else
    Allocator* ma = arena_allocator_create(4*1024);
#endif
    char* s1 = cstr_concat("","");
    char* extra = "abcedfghijklmnopqrstuvwxyz01923456789";
    for(int i = 0; i < 2800; i++) {
        char* s2 = cstr_concat(s1, extra);
        free(s1);
        s1 = s2;
    }
    CbufferRef b2 = Cbuffer_from_cstring(s1, ma);
    printf("b2 length %ld \n", Cbuffer_size(b2));
    printf("b2 m_size %ld \n", Cbuffer_size(b2));
    printf("b2 m_capacity %ld \n", Cbuffer_capacity(b2));
    printf("b2 m_cptr %lx \n", (long)Cbuffer_data(b2));
    UT_EQUAL_INT(2800*strlen(extra), Cbuffer_size(b2));
    Cbuffer_free(b2);
    b2 = NULL;
    UT_EQUAL_PTR(b2, NULL);
    free(s1);
    allocator_destroy(ma);
    return 0;
}
// demonstrate clear makes empty without additional allocation or deallocation
int test_cbuffer_clear()
{
#ifdef CBUF_ALLOCATOR_MALLOC
    Allocator* ma = malloc_allocator_create();
#else
    Allocator* ma = arena_allocator_create(4*1024);
#endif
    char* s1 = cstr_concat("","");
    char* extra = "abcedfghijklmnopqrstuvwxyz01923456789";
    for(int i = 0; i < 2800; i++) {
        char* s2 = cstr_concat(s1, extra);
        free(s1);
        s1 = s2;
    }
    CbufferRef b2 = Cbuffer_from_cstring(s1, ma);
    void* data1 = Cbuffer_data(b2);
    int sz1 = Cbuffer_size(b2);
    Cbuffer_clear(b2);
    void* data2 = Cbuffer_data(b2);
    int sz2 = Cbuffer_size(b2);
    UT_EQUAL_PTR(data1, data2);
    UT_NOT_EQUAL_INT(sz1, sz2);
    Cbuffer_free(b2);
    b2 = NULL;
    UT_EQUAL_PTR(b2, NULL);
    free(s1);
    allocator_destroy(ma);
    return 0;
}
#if 0
// C++ style move sematics
int test_cbuffer_move()
{
    char* s1 = cstr_concat("","");
    char* extra = "abcedfghijklmnopqrstuvwxyz01923456789";
    char* extra2 = "1234567890";
    for(int i = 0; i < 2800; i++) {
        char* s2 = cstr_concat(s1, extra);
        free(s1);
        s1 = s2;
    }
    CbufferRef b2 = Cbuffer_from_cstring(s1);
    void* d12 = Cbuffer_data(b2);
    int sz12 = Cbuffer_size(b2);
    CbufferRef b1 = Cbuffer_from_cstring(extra2);
    void* d11 = Cbuffer_data(b1);
    int sz11 = Cbuffer_size(b1);
    Cbuffer_move(b1, b2);
    void* d22 = Cbuffer_data(b2);
    int sz22 = Cbuffer_size(b2);
    void* d21 = Cbuffer_data(b1);
    int sz21 = Cbuffer_size(b1);

    UT_EQUAL_PTR(d21, d12);
    Cbuffer_free(b1); b1 = NULL;
    Cbuffer_free(b2); b2 = NULL;
    UT_EQUAL_PTR(b1, NULL);
    UT_EQUAL_PTR(b2, NULL);
    free(s1);
    return 0;
}
#endif
int main()
{
    UT_ADD(test_make_buffer);
    UT_ADD(test_expand);
    UT_ADD(test_expansion);
    UT_ADD(test_big_expansion);
    UT_ADD(test_cbuffer_clear);
    // UT_ADD(test_cbuffer_move);
    int rc = UT_RUN();
    return rc;
}