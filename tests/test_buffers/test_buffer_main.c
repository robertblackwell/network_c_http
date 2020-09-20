#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <c_eg/unittest.h>
#include <c_eg/buffer/contig_buffer.h>
#include <c_eg/buffer/buffer_chain.h>

char* cstr_concat(char* s1, char* s2)
{
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
    CBufferRef b = CBuffer_new();
    printf("m_size %ld \n", CBuffer_size(b));
    printf("m_capacity %ld \n", CBuffer_capacity(b));
    printf("m_cptr %lx \n", (long)CBuffer_data(b));
    UT_NOT_EQUAL_PTR(b, NULL);
    UT_EQUAL_INT(CBuffer_size(b), 0);
    UT_NOT_EQUAL_PTR((void*)CBuffer_data(b), NULL);
//    CBuffer_free(&b);
//    UT_EQUAL_PTR(b, NULL);
    return 0;
}
int test_expansion()
{
    char* s1 = cstr_concat("","");
    char* extra = "abcedfghijklmnopqrstuvwxyz01923456789";
    for(int i = 0; i < 5; i++) {
        char* s2 = cstr_concat(s1, extra);
        free(s1);
        s1 = s2;
    }

    CBufferRef b2 = CBuffer_from_cstring(s1);
    printf("b2 m_size %ld \n", CBuffer_size(b2));
    printf("b2 m_capacity %ld \n", CBuffer_capacity(b2));
    printf("b2 m_cptr %lx \n", (long)CBuffer_data(b2));
    UT_EQUAL_INT(5*strlen(extra), CBuffer_size(b2));
//    CBuffer_free(&b2);
//    UT_EQUAL_PTR(b2, NULL);
    return 0;
}
int test_big_expansion()
{
    char* s1 = cstr_concat("","");
    char* extra = "abcedfghijklmnopqrstuvwxyz01923456789";
    for(int i = 0; i < 2800; i++) {
        char* s2 = cstr_concat(s1, extra);
        free(s1);
        s1 = s2;
    }
    CBufferRef b2 = CBuffer_from_cstring(s1);
    printf("b2 length %ld \n", CBuffer_size(b2));
    printf("b2 m_size %ld \n", CBuffer_size(b2));
    printf("b2 m_capacity %ld \n", CBuffer_capacity(b2));
    printf("b2 m_cptr %lx \n", (long)CBuffer_data(b2));
    UT_EQUAL_INT(2800*strlen(extra), CBuffer_size(b2));
//    CBuffer_free(&b2);
//    UT_EQUAL_PTR(b2, NULL);
    return 0;
}
// demonstrate clear makes empty without additional allocation or deallocation
int test_cbuffer_clear()
{
    char* s1 = cstr_concat("","");
    char* extra = "abcedfghijklmnopqrstuvwxyz01923456789";
    for(int i = 0; i < 2800; i++) {
        char* s2 = cstr_concat(s1, extra);
        free(s1);
        s1 = s2;
    }
    CBufferRef b2 = CBuffer_from_cstring(s1);
    void* data1 = CBuffer_data(b2);
    int sz1 = CBuffer_size(b2);
    CBuffer_clear(b2);
    void* data2 = CBuffer_data(b2);
    int sz2 = CBuffer_size(b2);
    UT_EQUAL_PTR(data1, data2);
    UT_NOT_EQUAL_INT(sz1, sz2);
//    CBuffer_free(&b2);
//    UT_EQUAL_PTR(b2, NULL);
    return 0;
}
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
    CBufferRef b2 = CBuffer_from_cstring(s1);
    void* d12 = CBuffer_data(b2);
    int sz12 = CBuffer_size(b2);
    CBufferRef b1 = CBuffer_from_cstring(extra2);
    void* d11 = CBuffer_data(b1);
    int sz11 = CBuffer_size(b1);
    CBuffer_move(b1, b2);
    void* d22 = CBuffer_data(b2);
    int sz22 = CBuffer_size(b2);
    void* d21 = CBuffer_data(b1);
    int sz21 = CBuffer_size(b1);

    UT_EQUAL_PTR(d21, d12);
//    CBuffer_free(&b1);
//    CBuffer_free(&b2);
//    UT_EQUAL_PTR(b1, NULL);
//    UT_EQUAL_PTR(b2, NULL);

    return 0;
}
int test_chain_make()
{
    BufferChainRef bcr = BufferChain_new();
    char* s1 = cstr_concat("","");
    char* extra = "abcedfghijklmnopqrstuvwxyz01923456789";
    for(int i = 0; i < 2800; i++) {
        BufferChain_append(bcr, (void*)extra, strlen(extra));
    }
    UT_EQUAL_INT(BufferChain_size(bcr), 2800*strlen(extra))
    BufferChain_free(&bcr);
    UT_EQUAL_PTR(bcr, NULL);

    return 0;
}
int test_chain_compact() // and eq_cstr
{
    BufferChainRef bcr = BufferChain_new();
    char* s1 = cstr_concat("","");
    char* s2;
    char* extra = "abcedfghijklmnopqrstuvwxyz01923456789";
    for(int i = 0; i < 2800; i++) {
        BufferChain_append(bcr, (void*)extra, strlen(extra));
        s2 = cstr_concat(s1, extra);
        s1 = s2;
    }
    UT_EQUAL_INT(BufferChain_size(bcr), 2800*strlen(extra))
    CBufferRef c = BufferChain_compact(bcr);
    int x = strlen((char*)CBuffer_data(c));
    UT_EQUAL_INT(x, CBuffer_size(c));
    int y = strcmp(s1, (char*)CBuffer_data(c));
    bool ok = BufferChain_eq_cstr(bcr, s1);
    UT_EQUAL_INT(ok, 1);
    s1[3] = 'X';
    bool ok2 = BufferChain_eq_cstr(bcr, s1);
    UT_EQUAL_INT(ok2, 0);
    BufferChain_free(&bcr);
    UT_EQUAL_PTR(bcr, NULL);
//    CBuffer_free(c);
//    UT_EQUAL_PTR(c, NULL);
    return 0;
}

int main()
{
    UT_ADD(test_make_buffer);
    UT_ADD(test_expansion);
    UT_ADD(test_big_expansion);
    UT_ADD(test_cbuffer_clear);
    UT_ADD(test_cbuffer_move);
    UT_ADD(test_chain_make);
    UT_ADD(test_chain_compact);
    int rc = UT_RUN();
    return rc;
}