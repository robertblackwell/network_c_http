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

UrlRef Url_new(char* url);
void Url_free(UrlRef* this_ptr);
