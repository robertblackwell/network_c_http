#define _GNU_SOURCE
#include <c_http/url.h>
#include <c_http/buffer/cbuffer.h>

#include <assert.h>
#include <stdio.h>

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
    Cbuffer_free(&(this->scheme));
    Cbuffer_free(&(this->host));
    Cbuffer_free(&(this->port));
    Cbuffer_free(&(this->fragement));
    Cbuffer_free(&(this->path));
    Cbuffer_free(&(this->query));
    Cbuffer_free(&(this->user_info));
    eg_free(this);
    this = NULL;
}

