#define _GNU_SOURCE
#include <c_eg/url.h>
#include <c_eg/buffer/contig_buffer.h>

#include <http-parser/http_parser.h>
#include <assert.h>
#include <stdio.h>

Url* Url_new(char* url)
{
    struct http_parser_url u;
    http_parser_url_init(&u);
    http_parser_parse_url(url, strlen(url),0, &u);
    Url* this = eg_alloc(sizeof(Url_t));

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


void Url_free(Url** this_ptr)
{
    Url* this = *this_ptr;
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

