
#include <stdio.h>
#include <string.h>
#include <http_in_c/unittest.h>
#include <http_in_c/common/cbuffer.h>
#include <http_in_c/common/buffer_chain.h>
#include <http_in_c/common/iobuffer.h>

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
    Cbuffer_dispose(&b);
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
    Cbuffer_dispose(&b2);
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
    Cbuffer_dispose(&b2);
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
    Cbuffer_dispose(&b2);
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
    Cbuffer_dispose(&b1);
    Cbuffer_dispose(&b2);
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
    BufferChain_dispose(&bcr);
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
    IOBufferRef iob = BufferChain_compact(bcr);
//    int x = strlen((char*)IOBuffer_data(iob));
//    UT_EQUAL_INT(x, IOBuffer_data_len(iob));

    int y = strncmp(s1, (char*)IOBuffer_data(iob), strlen(s1));

    bool ok = BufferChain_eq_cstr(bcr, s1);
    UT_EQUAL_INT(ok, 1);
    s1[3] = 'X';
    bool ok2 = BufferChain_eq_cstr(bcr, s1);
    UT_EQUAL_INT(ok2, 0);
    BufferChain_dispose(&bcr);
    UT_EQUAL_PTR(bcr, NULL);
    IOBuffer_dispose(&iob);
    UT_EQUAL_PTR(iob, NULL);
    return 0;
}
BufferChainRef make_chain_1()
{
    char* str[5] = {
            (char*)"abcdefg",
            (char*)"hijklmno",
            (char*)"pqrstuvw",
            (char*)"xyz",
            NULL
    };
    BufferChainRef bc = BufferChain_new();
    for(int i = 0; i < 4; i++) {
        IOBufferRef iob = IOBuffer_from_cstring(str[i]);
        BufferChain_add_back(bc, iob);
    }
    return bc;
}
BufferChainRef make_chain_2()
{
    char* str[5] = {
            (char*)"ABCDEFGH",
            (char*)"IJKLMNOPQ",
            (char*)"RSTUVWXYZ1234",
            NULL
    };
    BufferChainRef bc = BufferChain_new();
    for(int i = 0; i < 3; i++) {
        IOBufferRef iob = IOBuffer_from_cstring(str[i]);
        BufferChain_add_back(bc, iob);
    }
    return bc;
}

int test_chain_front()
{
    BufferChainRef bc = make_chain_1();
    while(BufferChain_size(bc) != 0) {
        int size_1 = BufferChain_size(bc);
        IOBufferRef iob = BufferChain_pop_front(bc);
        int iob_size1 = IOBuffer_data_len(iob);
        int size_2 = BufferChain_size(bc);
        UT_EQUAL_INT(size_2, size_1 - iob_size1);
    }
    return 0;
}
int test_chain_steal()
{
    BufferChainRef bc1 = make_chain_1();
    BufferChainRef bc2 = make_chain_2();
    int size1 = BufferChain_size(bc1);
    int size2 = BufferChain_size(bc2);
    BufferChain_steal_bufferchain(bc1, bc2);
    UT_EQUAL_INT(BufferChain_size(bc1), size1 + size2);
    UT_EQUAL_INT(BufferChain_size(bc2), 0);                // notice this - this is the different between append and steal
    IOBufferRef compacted = BufferChain_compact(bc1);
    const char* s = IOBuffer_cstr(compacted);
    IOBuffer_dispose(&compacted);
    return 0;
}
int test_chain_append()
{
    BufferChainRef bc1 = make_chain_1();
    BufferChainRef bc2 = make_chain_2();
    int size1 = BufferChain_size(bc1);
    int size2 = BufferChain_size(bc2);
    BufferChain_append_bufferchain(bc1, bc2);
    UT_EQUAL_INT(BufferChain_size(bc1), size1 + size2);
    UT_EQUAL_INT(BufferChain_size(bc2), size2);                // notice this - this is the different between append and steal
    IOBufferRef compacted = BufferChain_compact(bc1);
    const char* s = IOBuffer_cstr(compacted);
    IOBuffer_dispose(&compacted);
    return 0;
}

IOBufferRef make_iobuffer()
{
    IOBufferRef iob = IOBuffer_from_cstring("abcdefghijklmnopqrstuvwxyz");
    IOBuffer_consume(iob, 10);
    return iob;
}
int test_iobuffer_dup()
{
    IOBufferRef iob = make_iobuffer();
    IOBufferRef iob_dup = IOBuffer_dup(iob);
    UT_EQUAL_INT(IOBuffer_data_len(iob), IOBuffer_data_len(iob_dup));
    UT_NOT_EQUAL_PTR(IOBuffer_data(iob), IOBuffer_data(iob_dup));

    UT_EQUAL_INT(IOBuffer_space_len(iob), IOBuffer_space_len(iob_dup));
    UT_NOT_EQUAL_PTR(IOBuffer_space(iob), IOBuffer_space(iob_dup));
    const char* s = IOBuffer_cstr(iob);
    const char* s_dup = IOBuffer_cstr(iob_dup);
    UT_EQUAL_INT(strcmp(s, s_dup), 0);
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
    IOBuffer_dispose(&ioref);
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
    char* io_datap_before;
    char* io_datap_after;
    char* datap_after;
    while(IOBuffer_data_len(ioref) > 0) {
        io_datap_before = IOBuffer_data(ioref);
        IOBuffer_consume(ioref, 1);
        io_datap_after = IOBuffer_data(ioref);
        /**
         * this is important what does IOBuffer_data() return when there is no data -
         * returns start of memory buffer NOT null
         */
        if(IOBuffer_data_len(ioref) == 0) {
            datap_after = data;
        } else {
            datap_after = data + i;
        }
        void* data2 = IOBuffer_data(ioref);
        int data_length_2 = IOBuffer_data_len(ioref);
        UT_EQUAL_PTR(io_datap_after, datap_after);
        UT_EQUAL_INT((data_length - i), IOBuffer_data_len(ioref));
        UT_EQUAL_INT(strncmp(&(sconst[i]), (char*)IOBuffer_data(ioref), IOBuffer_data_len(ioref)), 0);
        i++;
    }
    IOBuffer_dispose(&ioref);
    return 0;
}
int test_iobuffer_commit_consume_extra()
{
    char* s1 = "0987654321";
    char* s2 = "abcdefghijklmno";
    char* s3;
    int l = asprintf(&s3, "%s%s", s1,s2);

    IOBufferRef iob = IOBuffer_new_with_capacity(strlen(s3));

    // space pointer becomes data pointer after adding data
    void* p_start = IOBuffer_space(iob);
    IOBuffer_data_add(iob, s1, strlen(s1));
    void* tmp1 = IOBuffer_data(iob);
    UT_EQUAL_PTR(p_start, IOBuffer_data(iob));

    // after adding data IOBuffer_data_len() should give len of string added
    int len_1 = IOBuffer_data_len(iob);
    UT_EQUAL_INT(len_1, strlen(s1));

    // space pointer after one add shoud be start + len of string added
    void* p1 = (IOBuffer_data(iob) + len_1);
    void* space_after_1 = IOBuffer_space(iob);
    UT_EQUAL_PTR(p1, space_after_1);

    IOBuffer_data_add(iob, s2, strlen(s2));
    // after adding a second string designed to fill capacity space_len2 should be zero
    void* space2 = IOBuffer_space(iob);
    int space_len2 = IOBuffer_space_len(iob);
    UT_EQUAL_INT(space_len2, 0);
    // space pointer - not sure
    void* pc = (void*)(p_start + strlen(s1) + strlen(s2));
    UT_EQUAL_PTR(space2, pc);

    // data length should be sum of lengths of added strings
    int len_2 = IOBuffer_data_len(iob);
    UT_EQUAL_INT(len_2, strlen(s1) + strlen(s2));

    const char* str = IOBuffer_cstr(iob);

    UT_EQUAL_INT(0, strcmp(s3, str))
    // consume s1 should only leave s2
    IOBuffer_consume(iob, strlen(s1));
    UT_EQUAL_INT(strlen(s2), IOBuffer_data_len(iob));
    UT_EQUAL_INT(strcmp(IOBuffer_cstr(iob), s2), 0);

    // consume s2 should leave nothing
    IOBuffer_consume(iob, strlen(s2));
    UT_EQUAL_INT(IOBuffer_data_len(iob), 0);
    UT_EQUAL_PTR(IOBuffer_space(iob), p_start);

    // final test when buffer is empty data pointer should be p_start
    UT_EQUAL_PTR(p_start, IOBuffer_data(iob));
    printf("this is the end");
    return 0;
}

int main()
{
    UT_ADD(test_iobuffer_dup);
    UT_ADD(test_chain_append);
    UT_ADD(test_chain_steal);
    UT_ADD(test_chain_front);
    UT_ADD(test_iobuffer_commit_consume_extra);
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