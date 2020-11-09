#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <c_http/unittest.h>
#include <c_http/buffer/cbuffer.h>
#include <c_http/buffer/buffer_chain.h>
#include <c_http/buffer/iobuffer.h>

char* cstr_concat(char* s1, char* s2)
{
    int n1 = strlen(s1);
    int n2 = strlen(s2);
    int sz = strlen(s1) + strlen(s2);
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
    CbufferRef b = Cbuffer_new();
    printf("m_size %ld \n", Cbuffer_size(b));
    printf("m_capacity %ld \n", Cbuffer_capacity(b));
    printf("m_cptr %lx \n", (long)Cbuffer_data(b));
    UT_NOT_EQUAL_PTR(b, NULL);
    UT_EQUAL_INT(Cbuffer_size(b), 0);
    UT_NOT_EQUAL_PTR((void*)Cbuffer_data(b), NULL);
    Cbuffer_free(&b);
    UT_EQUAL_PTR(b, NULL);
    return 0;
}
int test_expansion()
{
    char* s1 = cstr_concat("","");
    char* extra = "abcedfghijklmnopqrstuvwxyz01923456789";
    CbufferRef b2 = Cbuffer_new();
    for(int i = 0; i < 5; i++) {
        Cbuffer_append(b2, (void*)extra, strlen(extra));
    }

    printf("b2 m_size %ld \n", Cbuffer_size(b2));
    printf("b2 m_capacity %ld \n", Cbuffer_capacity(b2));
    printf("b2 m_cptr %lx \n", (long)Cbuffer_data(b2));
    UT_EQUAL_INT(5*strlen(extra), Cbuffer_size(b2));
    Cbuffer_free(&b2);
    free(s1);
    UT_EQUAL_PTR(b2, NULL);
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
    CbufferRef b2 = Cbuffer_from_cstring(s1);
    printf("b2 length %ld \n", Cbuffer_size(b2));
    printf("b2 m_size %ld \n", Cbuffer_size(b2));
    printf("b2 m_capacity %ld \n", Cbuffer_capacity(b2));
    printf("b2 m_cptr %lx \n", (long)Cbuffer_data(b2));
    UT_EQUAL_INT(2800*strlen(extra), Cbuffer_size(b2));
    Cbuffer_free(&b2);
    UT_EQUAL_PTR(b2, NULL);
    free(s1);
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
    CbufferRef b2 = Cbuffer_from_cstring(s1);
    void* data1 = Cbuffer_data(b2);
    int sz1 = Cbuffer_size(b2);
    Cbuffer_clear(b2);
    void* data2 = Cbuffer_data(b2);
    int sz2 = Cbuffer_size(b2);
    UT_EQUAL_PTR(data1, data2);
    UT_NOT_EQUAL_INT(sz1, sz2);
    Cbuffer_free(&b2);
    UT_EQUAL_PTR(b2, NULL);
    free(s1);
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
    Cbuffer_free(&b1);
    Cbuffer_free(&b2);
    UT_EQUAL_PTR(b1, NULL);
    UT_EQUAL_PTR(b2, NULL);
    free(s1);
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
    int x = BufferChain_size(bcr);
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
    CbufferRef c = BufferChain_compact(bcr);
    int x = strlen((char*)Cbuffer_data(c));
    UT_EQUAL_INT(x, Cbuffer_size(c));
    int y = strcmp(s1, (char*)Cbuffer_data(c));
    bool ok = BufferChain_eq_cstr(bcr, s1);
    UT_EQUAL_INT(ok, 1);
    s1[3] = 'X';
    bool ok2 = BufferChain_eq_cstr(bcr, s1);
    UT_EQUAL_INT(ok2, 0);
    BufferChain_free(&bcr);
    UT_EQUAL_PTR(bcr, NULL);
    Cbuffer_free(&c);
    UT_EQUAL_PTR(c, NULL);
    return 0;
}
int test_iobuffer_make()
{
    IOBufferRef ioref = IOBuffer_new();
    void* x = IOBuffer_space(ioref);
    int l = IOBuffer_space_len(ioref);
    char* sconst = "A0123456789P";
    size_t y = strlen(sconst);
    memcpy(x, sconst, y+1);
    IOBuffer_commit(ioref, y+1);
    void* data = IOBuffer_data(ioref);
    int data_length = IOBuffer_data_len(ioref);
    IOBuffer_consume(ioref, 1);
    void* data_1 = IOBuffer_data(ioref);
    int data_length_1 = IOBuffer_data_len(ioref);
    UT_EQUAL_PTR((data+1), IOBuffer_data(ioref));
    UT_EQUAL_INT((data_length - 1), IOBuffer_data_len(ioref));
    UT_EQUAL_INT(strcmp("0123456789P", (char*)IOBuffer_data(ioref)), 0);
    IOBuffer_free(&ioref);
    return 0;
}
int test_iobuffer_make2()
{
    IOBufferRef ioref = IOBuffer_new();
    void* x = IOBuffer_space(ioref);
    int l = IOBuffer_space_len(ioref);
    char* sconst = "A0123456789P";
    size_t y = strlen(sconst);
    memcpy(x, sconst, y); // this time dont copy the zero terminator
    IOBuffer_commit(ioref, y);
    void* data = IOBuffer_data(ioref);
    int data_length = IOBuffer_data_len(ioref);
    int i = 1;
    while(IOBuffer_data_len(ioref) > 0) {
        IOBuffer_consume(ioref, 1);
        void* data2 = IOBuffer_data(ioref);
        int data_length_2 = IOBuffer_data_len(ioref);
        UT_EQUAL_PTR((data+i), IOBuffer_data(ioref));
        UT_EQUAL_INT((data_length - i), IOBuffer_data_len(ioref));
        UT_EQUAL_INT(strncmp(&(sconst[i]), (char*)IOBuffer_data(ioref), IOBuffer_data_len(ioref)), 0);
        i++;
    }
    IOBuffer_free(&ioref);
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
    UT_ADD(test_iobuffer_make);
    UT_ADD(test_iobuffer_make2);
    int rc = UT_RUN();
    return rc;
}