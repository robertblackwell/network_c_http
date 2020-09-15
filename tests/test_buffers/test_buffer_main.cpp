
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <c_eg/buffer/contig_buffer.h>
#include <string>
TEST_CASE("buffer_chain_assignment")
{
    printf("XXXXXXXXXXXHello world from buffer test \n");
    CHECK(1 == 1);
}
TEST_CASE("make a buffer - basics")
{
    CBufferRef b = CBuffer_new();
    printf("m_size %ld \n", CBuffer_size(b));
    printf("m_capacity %ld \n", CBuffer_capacity(b));
    printf("m_cptr %lx \n", (long)CBuffer_data(b));
    CHECK(b != nullptr);
    CHECK(CBuffer_size(b) == 0);
    CHECK( ((void*)CBuffer_data(b)) != nullptr  );
}
TEST_CASE("make a buffer from a string no expansion")
{
    std::string s1="";
    for(int i = 0; i < 5; i++) {
        s1 = s1 + "abcedfghijklmnopqrstuvwxyz01923456789";
    }
    char* c1 = (char*) s1.c_str();
    CBufferRef b2 = CBuffer_from_string(c1);
    printf("b2 length %ld \n", s1.size());
    printf("b2 m_size %ld \n", CBuffer_size(b2));
    printf("b2 m_capacity %ld \n", CBuffer_capacity(b2));
    printf("b2 m_cptr %lx \n", (long)CBuffer_data(b2));
    CHECK(s1.size() == CBuffer_size(b2));
}
TEST_CASE("make a buffer from a string with expansion expansion")
{
    std::string s1="";
    for(int i = 0; i < 11; i++) {
        s1 = s1 + "abcedfghijklmnopqrstuvwxyz01923456789";
    }
    char* c1 = (char*) s1.c_str();
    CBufferRef b2 = CBuffer_from_string(c1);
    printf("b2 length %ld \n", s1.size());
    printf("b2 m_size %ld \n", CBuffer_size(b2));
    printf("b2 m_capacity %ld \n", CBuffer_capacity(b2));
    printf("b2 m_cptr %lx \n", (long)CBuffer_data(b2));
    CHECK(s1.size() == CBuffer_size(b2));
}
TEST_CASE("make a buffer from a string with a big expansion expansion")
{
    std::string s1="";
    for(int i = 0; i < 2800; i++) {
        s1 = s1 + "abcedfghijklmnopqrstuvwxyz01923456789";
    }
    char* c1 = (char*) s1.c_str();
    CBufferRef b2 = CBuffer_from_string(c1);
    printf("b2 length %ld \n", s1.size());
    printf("b2 m_size %ld \n", CBuffer_size(b2));
    printf("b2 m_capacity %ld \n", CBuffer_capacity(b2));
    printf("b2 m_cptr %lx \n", (long)CBuffer_data(b2));
    CHECK(s1.size() == CBuffer_size(b2));
}
TEST_CASE("make a buffer from a string and then append to it")
{
    CBufferRef b2 = CBuffer_new();
    char* s = (char*)std::string("abcedfghijklmnopqrstuvwxyz01923456789").c_str();
    for(int i = 0; i < 20; i++) {
        CHECK(CBuffer_size(b2) == strlen(s) * i);
        CBuffer_append_cstr(b2, s);
        CHECK(CBuffer_size(b2) == strlen(s) * (i+1));
    }
    printf("b2 m_size %ld \n", CBuffer_size(b2));
    printf("b2 m_capacity %ld \n", CBuffer_capacity(b2));
    printf("b2 m_cptr %lx \n", (long)CBuffer_data(b2));
}