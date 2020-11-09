#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <http-parser/http_parser.h>
#include <c_eg/alloc.h>
#include <c_eg/unittest.h>
#include <c_eg/buffer/cbuffer.h>
#include <c_eg/logger.h>
#include <c_eg/list.h>
#include <c_eg/server.h>
#include <c_eg/hdrlist.h>
#include <c_eg/message.h>

//https://github.com/uriparser/uriparser

typedef struct Url_s {
    CbufferRef scheme;
    CbufferRef host;
    CbufferRef path;
    CbufferRef port;
    CbufferRef fragement;
    CbufferRef query;
    CbufferRef user_info;

} Url_t, Url;

UrlRef Url_new(char* url);
void Url_free(UrlRef* this_ptr);
