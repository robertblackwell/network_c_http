#define _GNU_SOURCE
#include <c_eg/message.h>
#include <c_eg/alloc.h>
#include <c_eg/hdrlist.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

struct Message_s
{
    BufferChain* body;
    HdrList* headers;
    int major_vers;
    HttpMinorVersion minor_vers;
    bool is_request;
    struct {
        HttpStatus status_code;
        Cbuffer* reason;
    };
    struct {
        HttpMethod method;
        Cbuffer* target;
    };
};

Message* Message_new ()
{
    Message* mref = (Message*) eg_alloc(sizeof(Message));
    if(mref == NULL) goto error_label_1;

    mref->body = NULL;
    mref->headers = HdrList_new();
    if(mref->headers == NULL) goto error_label_2;
    mref->minor_vers = minor_version1;
    mref->major_vers = major_version1;
    mref->target = Cbuffer_new();
    mref->reason = Cbuffer_new();
    return mref;

    error_label_2:
        Message_free(&mref);
    error_label_1:
        return NULL;
}
Message* Message_new_request()
{
    Message* mref = Message_new();
    if(mref != NULL) {
        mref->is_request = true;
//        mref->target = Cbuffer_new();
        return mref;
    }
    return NULL;
}
Message* Message_new_response()
{

    Message* mref = Message_new();
    if(mref != NULL) {
        mref->is_request = false;
//        mref->reason = Cbuffer_new();
        return mref;
    }
    return NULL;
}
void Message_free(Message** this_p)
{
    Message* this = *this_p;
    HdrList_free(&(this->headers));
    Cbuffer_free(&(this->target));
    Cbuffer_free(&(this->reason));

    eg_free(*this_p);
    *this_p = NULL;
}
void Message_dispose(void* p)
{
    Message_free((Message**)&p);
}
Message* MessageResponse(HttpStatus status, void* body)
{
    Message* mref = Message_new(false);
    if(mref == NULL) goto error_1;
    mref->is_request = false;
    mref->status_code = status;
    mref->body = body;
    if(mref->target != NULL) Cbuffer_free(&(mref->target));
    if(mref->reason != NULL) Cbuffer_free(&(mref->reason));
    return mref;
    error_1:
        return NULL;
}
Cbuffer* Message_serialize_request(Message* mref)
{
    Cbuffer* result = Cbuffer_new();
    char* first_line;
    char* meth = http_method_str(mref->method);

    int l1= asprintf(&first_line,"%s %s HTTP/%d.%d\r\n", meth, (char*)Cbuffer_data(mref->target), mref->major_vers, mref->minor_vers);
    Cbuffer_append(result, (void*)first_line, l1);
    free(first_line);
    HdrList* hdrs = mref->headers;
    ListIterator iter = HdrList_iterator(hdrs);
    while(iter != NULL) {
        KVPair* item = HdrList_itr_unpack(hdrs, iter);
        char* s;
        int len = asprintf(&s,"%s: %s\r\n", KVPair_label(item), KVPair_value(item));
        Cbuffer_append(result, (void*)s, len);
        ListIterator next = HdrList_itr_next(hdrs, iter);
        iter = next;
        free(s);
    }
    Cbuffer_append_cstr(result, "\r\n");
    return result;
}
Cbuffer* Message_serialize_response(Message* mref)
{
    Cbuffer* result = Cbuffer_new();
    char* first_line;
    int ll = asprintf(&first_line, "HTTP/%d.%d  %d %s\r\n",mref->major_vers, mref->minor_vers, mref->status_code, (char*)Cbuffer_data(mref->reason));
    Cbuffer_append(result, (void*)first_line, ll);
    free(first_line);
    HdrList* hdrs = mref->headers;
    ListIterator iter = HdrList_iterator(hdrs);
    while(iter != NULL) {
        KVPair* item = HdrList_itr_unpack(hdrs, iter);
        char* s;
        int len = asprintf(&s,"%s: %s\r\n", KVPair_label(item), KVPair_value(item));
        Cbuffer_append(result, (void*)s, len);
        ListIterator next = HdrList_itr_next(hdrs, iter);
        iter = next;
        free(s);
    }
    Cbuffer_append_cstr(result, "\r\n");
    Cbuffer* body = BufferChain_compact(Message_get_body(mref));
    Cbuffer_append(result, Cbuffer_data(body), Cbuffer_size(body));
    return result;
}
Cbuffer* Message_serialize(Message* mref)
{
    Cbuffer* result;
    if(mref->is_request) {
        result = Message_serialize_request(mref);
    } else {
        result = Message_serialize_response(mref);
    }
    return result;
}

HttpStatus Message_get_status(Message* this)
{
    return this->status_code;
}
void Message_set_status(Message* this, HttpStatus status)
{
    this->status_code = status;
}
bool Message_get_is_request(Message* this)
{
    return this->is_request;
}
void Message_set_is_request(Message* this, bool yn)
{
    this->is_request = yn;
}
HttpMinorVersion Message_get_minor_version(Message* this)
{
    return this->minor_vers;
}

void Message_set_minor_version(Message* this, HttpMinorVersion mv)
{
    this->minor_vers = mv;
}
void Message_set_version(Message* this, int major_vers, int minor_vers)
{
    this->minor_vers = minor_vers;
}
void Message_set_method(Message* this, HttpMethod method)
{
    this->method = method;
}
HttpMethod Message_get_method(Message* this)
{
    return this->method;
}
Cbuffer* Message_get_target(Message* this)
{
    return this->target;
}
void Message_move_target(Message* this, Cbuffer* target)
{
    if(this->target == NULL) {
        this->target = Cbuffer_new();
    }
    Cbuffer_move(this->target, target);
}
void Message_set_target(Message* this, char* targ)
{
    assert((this->target != NULL) && (Cbuffer_size(this->target) == 0));
    Cbuffer_append_cstr(this->target, targ);
}
void Message_move_reason(Message* this, Cbuffer* reason)
{
    if(this->reason == NULL)
        this->reason = Cbuffer_new();
    Cbuffer_move(this->reason, reason);
}
char* Message_get_reason(Message* this)
{
    (char*)Cbuffer_data(this->reason);
}
HdrList* Message_headers(Message* this)
{
    return this->headers;
}
BufferChain* Message_get_body(Message* this)
{
    return this->body;
}
void Message_set_body(Message* this, BufferChain* bc)
{
    this->body = bc;
}
