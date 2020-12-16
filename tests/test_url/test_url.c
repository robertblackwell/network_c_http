#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <c_http/api/url_parser.h>
#include <c_http/dsl/alloc.h>
#include <c_http/unittest.h>
#include <c_http/api/cbuffer.h>
#include <c_http/logger.h>
#include <c_http/dsl/list.h>
#include <c_http/api/server.h>
#include <c_http/details/hdrlist.h>
#include <c_http/api/message.h>

//https://github.com/uriparser/uriparser

typedef struct Url_s {
    CbufferRef scheme;
    CbufferRef host;
    CbufferRef path;
    CbufferRef port;
    CbufferRef fragement;
    CbufferRef query;
    CbufferRef user_info;

} Url_t,  Url, *UrlRef;

UrlRef Url_new(char* url)
{
    struct c_http_parser_url u;
    c_http_parser_url_init(&u);
    c_http_parser_parse_url(url, strlen(url),0, &u);
    UrlRef this = eg_alloc(sizeof(Url_t));

    this->scheme = Cbuffer_new();
    if(u.field_data[UF_SCHEMA].len != 0)
        Cbuffer_append(this->scheme, u.field_data[UF_SCHEMA].off + url, u.field_data[UF_SCHEMA].len);

    this->user_info = Cbuffer_new();
    if(u.field_data[UF_USERINFO].len != 0)
        Cbuffer_append(this->user_info, u.field_data[UF_USERINFO].off + url, u.field_data[UF_USERINFO].len);

    this->host = Cbuffer_new();
    if(u.field_data[UF_HOST].len != 0)
        Cbuffer_append(this->host, u.field_data[UF_HOST].off + url, u.field_data[UF_HOST].len);

    this->path = Cbuffer_new();
    if(u.field_data[UF_PATH].len != 0)
    Cbuffer_append(this->path, u.field_data[UF_PATH].off + url, u.field_data[UF_PATH].len);

    this->port = Cbuffer_new();
    if(u.field_data[UF_PORT].len != 0)
        Cbuffer_append(this->port, u.field_data[UF_PORT].off + url, u.field_data[UF_PORT].len);

    this->fragement = Cbuffer_new();
    if(u.field_data[UF_FRAGMENT].len != 0)
    Cbuffer_append(this->fragement, u.field_data[UF_FRAGMENT].off + url, u.field_data[UF_FRAGMENT].len);

    this->query = Cbuffer_new();
    if(u.field_data[UF_QUERY].len != 0)
    Cbuffer_append(this->query, u.field_data[UF_QUERY].off + url, u.field_data[UF_QUERY].len);

    return this;

}

void Url_free(UrlRef* this_ptr)
{
    UrlRef this = *this_ptr;
    Cbuffer_dispose(&(this->scheme));
    Cbuffer_dispose(&(this->host));
    Cbuffer_dispose(&(this->port));
    Cbuffer_dispose(&(this->fragement));
    Cbuffer_dispose(&(this->path));
    Cbuffer_dispose(&(this->query));
    Cbuffer_dispose(&(this->user_info));
    eg_free(this);
    this = NULL;
}

int xtest_url_01()
{
    char* url = "http://www.somewhere.com/path1/path2?a=1111&b=2222";
    struct c_http_parser_url u;
    c_http_parser_url_init(&u);
    c_http_parser_parse_url(url, strlen(url),0, &u);
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
    char* scheme = (char*)Cbuffer_data(uref->scheme);
    char* host = (char*)Cbuffer_data(uref->host);

    UT_EQUAL_INT(strcmp(Cbuffer_data(uref->scheme), "http"), 0);
    UT_EQUAL_INT(strcmp(Cbuffer_data(uref->host), "www.somewhere.com"), 0);
    UT_EQUAL_INT(strcmp(Cbuffer_data(uref->path), "/path1/path2"), 0);
    UT_EQUAL_INT(strcmp(Cbuffer_data(uref->port), ""), 0);
    UT_EQUAL_INT(strcmp(Cbuffer_data(uref->fragement), ""), 0);
    UT_EQUAL_INT(strcmp(Cbuffer_data(uref->query), "a=1111&b=2222"), 0);
    return 0;
}

int main()
{
    UT_ADD(test_url_01);
    int rc = UT_RUN();
    return rc;
}