#define _GNU_SOURCE
#include <c_http/message.h>
#include <c_http/alloc.h>
#include <c_http/hdrlist.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

struct Message_s
{
    BufferChainRef body;
    HdrListRef headers;
    int major_vers;
    HttpMinorVersion minor_vers;
    bool is_request;
    struct {
        HttpStatus status_code;
        CbufferRef reason;
    };
    struct {
        HttpMethod method;
        CbufferRef target;
    };
};

MessageRef Message_new ()
{
    MessageRef mref = (MessageRef) eg_alloc(sizeof(Message));
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
MessageRef Message_new_request()
{
    MessageRef mref = Message_new();
    if(mref != NULL) {
        mref->is_request = true;
//        mref->target = Cbuffer_new();
        return mref;
    }
    return NULL;
}
MessageRef Message_new_response()
{

    MessageRef mref = Message_new();
    if(mref != NULL) {
        mref->is_request = false;
//        mref->reason = Cbuffer_new();
        return mref;
    }
    return NULL;
}
void Message_free(MessageRef* this_p)
{
    MessageRef this = *this_p;
    HdrList_free(&(this->headers));
    Cbuffer_free(&(this->target));
    Cbuffer_free(&(this->reason));

    eg_free(*this_p);
    *this_p = NULL;
}
void Message_dispose(void* p)
{
    Message_free((MessageRef*)&p);
}
MessageRef MessageResponse(HttpStatus status, void* body)
{
    MessageRef mref = Message_new(false);
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
CbufferRef Message_serialize_request(MessageRef mref)
{
    CbufferRef result = Cbuffer_new();
    char* first_line;
    const char* meth = http_method_str(mref->method);

    int l1= asprintf(&first_line,"%s %s HTTP/%d.%d\r\n", meth, (char*)Cbuffer_data(mref->target), mref->major_vers, mref->minor_vers);
    Cbuffer_append(result, (void*)first_line, l1);
    free(first_line);
    HdrListRef hdrs = mref->headers;
    ListIterator iter = HdrList_iterator(hdrs);
    while(iter != NULL) {
        KVPairRef item = HdrList_itr_unpack(hdrs, iter);
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
CbufferRef Message_serialize_response(MessageRef mref)
{
    CbufferRef result = Cbuffer_new();
    char* first_line;
    int ll = asprintf(&first_line, "HTTP/%d.%d  %d %s\r\n",mref->major_vers, mref->minor_vers, mref->status_code, (char*)Cbuffer_data(mref->reason));
    Cbuffer_append(result, (void*)first_line, ll);
    free(first_line);
    HdrListRef hdrs = mref->headers;
    ListIterator iter = HdrList_iterator(hdrs);
    while(iter != NULL) {
        KVPairRef item = HdrList_itr_unpack(hdrs, iter);
        char* s;
        int len = asprintf(&s,"%s: %s\r\n", KVPair_label(item), KVPair_value(item));
        Cbuffer_append(result, (void*)s, len);
        ListIterator next = HdrList_itr_next(hdrs, iter);
        iter = next;
        free(s);
    }
    Cbuffer_append_cstr(result, "\r\n");
    CbufferRef body = BufferChain_compact(Message_get_body(mref));
    Cbuffer_append(result, Cbuffer_data(body), Cbuffer_size(body));
    return result;
}
CbufferRef Message_serialize(MessageRef mref)
{
    CbufferRef result;
    if(mref->is_request) {
        result = Message_serialize_request(mref);
    } else {
        result = Message_serialize_response(mref);
    }
    return result;
}

HttpStatus Message_get_status(MessageRef this)
{
    return this->status_code;
}
void Message_set_status(MessageRef this, HttpStatus status)
{
    this->status_code = status;
}
bool Message_get_is_request(MessageRef this)
{
    return this->is_request;
}
void Message_set_is_request(MessageRef this, bool yn)
{
    this->is_request = yn;
}
HttpMinorVersion Message_get_minor_version(MessageRef this)
{
    return this->minor_vers;
}

void Message_set_minor_version(MessageRef this, HttpMinorVersion mv)
{
    this->minor_vers = mv;
}
void Message_set_version(MessageRef this, int major_vers, int minor_vers)
{
    this->minor_vers = minor_vers;
}
void Message_set_method(MessageRef this, HttpMethod method)
{
    this->method = method;
}
HttpMethod Message_get_method(MessageRef this)
{
    return this->method;
}
CbufferRef Message_get_target(MessageRef this)
{
    return this->target;
}
void Message_move_target(MessageRef this, CbufferRef target)
{
    if(this->target == NULL) {
        this->target = Cbuffer_new();
    }
    Cbuffer_move(this->target, target);
}
void Message_set_target(MessageRef this, char* targ)
{
    assert((this->target != NULL) && (Cbuffer_size(this->target) == 0));
    Cbuffer_append_cstr(this->target, targ);
}
void Message_move_reason(MessageRef this, CbufferRef reason)
{
    if(this->reason == NULL)
        this->reason = Cbuffer_new();
    Cbuffer_move(this->reason, reason);
}
char* Message_get_reason(MessageRef this)
{
    (char*)Cbuffer_data(this->reason);
}
HdrListRef Message_headers(MessageRef this)
{
    return this->headers;
}
BufferChainRef Message_get_body(MessageRef this)
{
    return this->body;
}
void Message_set_body(MessageRef this, BufferChainRef bc)
{
    this->body = bc;
}
