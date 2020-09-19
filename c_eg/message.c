#include <c_eg/message.h>
#include <stdbool.h>
#include <stdlib.h>
#include <c_eg/headerline_list.h>

struct Message_s
{
    void *body;
    HDRListRef headers;
    int major_vers;
    HttpMinorVersion minor_vers;
    bool is_request;
    union
    {
        struct {
            HttpStatus status_code;
            CBufferRef reason;
        };
        struct {
            HttpMethod method;
            CBufferRef target;
        };
    };
};

MessageRef Message_new ()
{
    MessageRef mref = (MessageRef) malloc(sizeof(Message));
    if(mref == NULL) goto error_label_1;

    mref->body = NULL;
    mref->headers = HDRList_new();
    if(mref->headers == NULL) goto error_label_2;
    mref->minor_vers = minor_version1;
    mref->major_vers = major_version1;

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
        return mref;
    }
    return NULL;
}
MessageRef Message_new_response()
{

    MessageRef mref = Message_new();
    if(mref != NULL) {
        mref->is_request = false;
        return mref;
    }
    return NULL;
}
void Message_free(MessageRef* this_p)
{
    free(*this_p);
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
    return mref;
    error_1:
        return NULL;
}
char* Message_serialize_response(MessageRef mref)
{
    return "";
}
char* Message_serialize_request(MessageRef mref)
{
    return "";
}
char* Message_serialize(MessageRef mref)
{
    char* result;
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
void Message_move_target(MessageRef this, CBufferRef target)
{
    CBuffer_move(this->target, target);
}
void Message_move_reason(MessageRef this, CBufferRef reason)
{
    if(this->reason == NULL)
        this->reason = CBuffer_new();
    CBuffer_move(this->reason, reason);
}
char* Message_get_reason(MessageRef this)
{
    (char*)CBuffer_data(this->reason);
}
HDRListRef Message_headers(MessageRef this)
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