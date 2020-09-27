#define _GNU_SOURCE
#include <c_eg/message.h>
#include <c_eg/alloc.h>
#include <c_eg/hdrlist.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
struct Message_s
{
    BufferChainRef body;
    HdrList* headers;
    int major_vers;
    HttpMinorVersion minor_vers;
    bool is_request;
    struct {
        HttpStatus status_code;
        CBufferRef reason;
    };
    struct {
        HttpMethod method;
        CBufferRef target;
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
    mref->target = CBuffer_new();
    mref->reason = CBuffer_new();
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
//        mref->target = CBuffer_new();
        return mref;
    }
    return NULL;
}
Message* Message_new_response()
{

    Message* mref = Message_new();
    if(mref != NULL) {
        mref->is_request = false;
//        mref->reason = CBuffer_new();
        return mref;
    }
    return NULL;
}
void Message_free(Message** this_p)
{
    Message* this = *this_p;
    HdrList_free(&(this->headers));
    CBuffer_free(&(this->target));
    CBuffer_free(&(this->reason));

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
    if(mref->target != NULL) CBuffer_free(&(mref->target));
    if(mref->reason != NULL) CBuffer_free(&(mref->reason));
    return mref;
    error_1:
        return NULL;
}
CBufferRef Message_serialize_request(Message* mref)
{
    CBufferRef result = CBuffer_new();
    char* first_line;
    char* meth = "METH";
    int l1= asprintf(&first_line,"%s %s HTTP/%d.%d\r\n", meth, (char*)CBuffer_data(mref->target), mref->major_vers, mref->minor_vers);
    CBuffer_append(result, (void*)first_line, l1);
    free(first_line);
    HdrList* hdrs = mref->headers;
    ListIterator iter = HdrList_iterator(hdrs);
    while(iter != NULL) {
        KVPair* item = HdrList_itr_unpack(hdrs, iter);
        char* s;
        int len = asprintf(&s,"%s: %s\r\n", KVPair_label(item), KVPair_value(item));
        CBuffer_append(result, (void*)s, len);
        ListIterator next = HdrList_itr_next(hdrs, iter);
        iter = next;
        free(s);
    }
    CBuffer_append_cstr(result, "\r\n");
    return result;
}
CBufferRef Message_serialize_response(Message* mref)
{
    char* first_line;
    char* meth = "METH";
    asprintf(&first_line, "%s %s HTTP/%d.%d\r\n", meth, (char*)CBuffer_data(mref->target), mref->major_vers, mref->minor_vers);
    return NULL;
}
CBufferRef Message_serialize(Message* mref)
{
    CBufferRef result;
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
CBufferRef Message_get_target(Message* this)
{
    return this->target;
}
void Message_move_target(Message* this, CBufferRef target)
{
    if(this->target == NULL) {
        this->target = CBuffer_new();
    }
    CBuffer_move(this->target, target);
}
void Message_move_reason(Message* this, CBufferRef reason)
{
    if(this->reason == NULL)
        this->reason = CBuffer_new();
    CBuffer_move(this->reason, reason);
}
char* Message_get_reason(Message* this)
{
    (char*)CBuffer_data(this->reason);
}
HdrList* Message_headers(Message* this)
{
    return this->headers;
}
BufferChainRef Message_get_body(Message* this)
{
    return this->body;
}
void Message_set_body(Message* this, BufferChainRef bc)
{
    this->body = bc;
}
