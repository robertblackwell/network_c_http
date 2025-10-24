
#include <assert.h>
#include <stdio.h>
#include <src/common/cbuffer.h>
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
void Url_dispose(UrlRef* this_ptr);
