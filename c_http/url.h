#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <c_http/url_parser.h>
#include <c_http/alloc.h>
#include <c_http/unittest.h>
#include <c_http/buffer/cbuffer.h>
#include <c_http/logger.h>
#include <c_http/list.h>
#include <c_http/server.h>
#include <c_http/hdrlist.h>
#include <c_http/message.h>

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
