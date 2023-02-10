
#include <http_in_c/common/url.h>
#include <http_in_c/common/alloc.h>
#include <http_in_c/common/url_parser.h>
#include <http_in_c/common/cbuffer.h>
#include <string.h>

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


void Url_dispose(UrlRef* this_ptr)
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

