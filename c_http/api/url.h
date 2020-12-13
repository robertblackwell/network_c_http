#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <c_http/api/url_parser.h>
#include <c_http/dsl/alloc.h>
#include <c_http/unittest.h>
#include <c_http/dsl/cbuffer.h>
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

} Url_t, Url, *UrlRef;

UrlRef Url_new(char* url);
void Url_free(UrlRef* this_ptr);
