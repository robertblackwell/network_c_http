#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <http-parser/http_parser.h>
#include <c_eg/alloc.h>
#include <c_eg/unittest.h>
#include <c_eg/buffer/contig_buffer.h>
#include <c_eg/logger.h>
#include <c_eg/list.h>
#include <c_eg/server.h>
#include <c_eg/hdrlist.h>
#include <c_eg/message.h>

//https://github.com/uriparser/uriparser

typedef struct Url_s {
    CBufferRef scheme;
    CBufferRef host;
    CBufferRef path;
    CBufferRef port;
    CBufferRef fragement;
    CBufferRef query;
    CBufferRef user_info;

} Url_t,  *UrlRef;

UrlRef Url_new(char* url)
{
    struct http_parser_url u;
    http_parser_url_init(&u);
    http_parser_parse_url(url, strlen(url),0, &u);
    UrlRef this = eg_alloc(sizeof(Url_t));

    this->scheme = CBuffer_new();
    if(u.field_data[UF_SCHEMA].len != 0)
        CBuffer_append(this->scheme, u.field_data[UF_SCHEMA].off + url, u.field_data[UF_SCHEMA].len);

    this->user_info = CBuffer_new();
    if(u.field_data[UF_USERINFO].len != 0)
        CBuffer_append(this->user_info, u.field_data[UF_USERINFO].off + url, u.field_data[UF_USERINFO].len);

    this->host = CBuffer_new();
    if(u.field_data[UF_HOST].len != 0)
        CBuffer_append(this->host, u.field_data[UF_HOST].off + url, u.field_data[UF_HOST].len);

    this->path = CBuffer_new();
    if(u.field_data[UF_PATH].len != 0)
    CBuffer_append(this->path, u.field_data[UF_PATH].off + url, u.field_data[UF_PATH].len);

    this->port = CBuffer_new();
    if(u.field_data[UF_PORT].len != 0)
        CBuffer_append(this->port, u.field_data[UF_PORT].off + url, u.field_data[UF_PORT].len);

    this->fragement = CBuffer_new();
    if(u.field_data[UF_FRAGMENT].len != 0)
    CBuffer_append(this->fragement, u.field_data[UF_FRAGMENT].off + url, u.field_data[UF_FRAGMENT].len);

    this->query = CBuffer_new();
    if(u.field_data[UF_QUERY].len != 0)
    CBuffer_append(this->query, u.field_data[UF_QUERY].off + url, u.field_data[UF_QUERY].len);

    return this;

}


void Url_free(UrlRef* this_ptr)
{
    UrlRef this = *this_ptr;
    CBuffer_free(&(this->scheme));
    CBuffer_free(&(this->host));
    CBuffer_free(&(this->port));
    CBuffer_free(&(this->fragement));
    CBuffer_free(&(this->path));
    CBuffer_free(&(this->query));
    CBuffer_free(&(this->user_info));
    eg_free(this);
    this = NULL;
}

int xtest_url_01()
{
    char* url = "http://www.somewhere.com/path1/path2?a=1111&b=2222";
    struct http_parser_url u;
    http_parser_url_init(&u);
    http_parser_parse_url(url, strlen(url),0, &u);
    char* scheme = url + u.field_data[UF_SCHEMA].off;

    char* host = url + u.field_data[UF_HOST].off;
    char* path = url + u.field_data[UF_PATH].off;
    char* port = url + u.field_data[UF_PORT].off;
    char* fragment = url + u.field_data[UF_FRAGMENT].off;
    char* query = url + u.field_data[UF_QUERY].off;
    char* user_info = url + u.field_data[UF_USERINFO].off;

    UrlRef uref = Url_new(url);
    return 0;
}
int test_url_01()
{
    char* url = "http://www.somewhere.com/path1/path2?a=1111&b=2222";
    UrlRef uref = Url_new(url);
    char* scheme = (char*)CBuffer_data(uref->scheme);
    char* host = (char*)CBuffer_data(uref->host);

    UT_EQUAL_INT(strcmp(CBuffer_data(uref->scheme), "http"), 0);
    UT_EQUAL_INT(strcmp(CBuffer_data(uref->host), "www.somewhere.com"), 0);
    UT_EQUAL_INT(strcmp(CBuffer_data(uref->path), "/path1/path2"), 0);
    UT_EQUAL_INT(strcmp(CBuffer_data(uref->port), ""), 0);
    UT_EQUAL_INT(strcmp(CBuffer_data(uref->fragement), ""), 0);
    UT_EQUAL_INT(strcmp(CBuffer_data(uref->query), "a=1111&b=2222"), 0);
    return 0;
}

int main()
{
    UT_ADD(test_url_01);
    int rc = UT_RUN();
    return rc;
}