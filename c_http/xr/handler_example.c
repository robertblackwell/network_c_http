#define _GNU_SOURCE
#include <c_http/xr/handler_example.h>

#include <c_http/constants.h>
#include <c_http/alloc.h>
#include <c_http/utils.h>
#include <c_http/socket_functions.h>
#include <c_http/message.h>
#include <c_http/ll_parser.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

#include <pthread.h>


static char* simple_response_body(char* message, socket_handle_t socket, int pthread_self_value)
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char dt_s[64];
    assert(strftime(dt_s, sizeof(dt_s), "%c", tm));

    char* body = "<html>"
                 "<head>"
                 "</head>"
                 "<body>"
                 "%s"
                 "<p>Date/Time is %s</p>"
                 "<p>socket: %d</p>"
                 "<p>p_thread_self %ld</p>"
                 "</body>"
                 "</html>";

    char* s1;
    int len1 = asprintf(&s1, body, message, dt_s, socket, pthread_self_value);

    return s1;
}
static CbufferRef XrSimpleBody(XrHandlerRef this, int fd)
{
    CbufferRef mserialized = Message_serialize(this->request);
    char* msg = Cbuffer_cstr(mserialized);

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char dt_s[64];
    assert(strftime(dt_s, sizeof(dt_s), "%c", tm));

    char* body = "<html>"
                 "<head>"
                 "</head>"
                 "<body>"
                 "%s"
                 "<p>Date/Time is %s</p>"
                 "<p>socket: %d</p>"
                 "<p>p_thread_self %ld</p>"
                 "</body>"
                 "</html>";

    char* s1;
    int len1 = asprintf(&s1, body, msg, dt_s, fd);
    CbufferRef cbuf = Cbuffer_from_cstring(s1);
    free(s1);
    Cbuffer_free(&mserialized);
    return cbuf;
}

/**
 * Create the body of a response to an /echo request
 * \param request
 * \return char* ownership, and responsibility to free, transfers to the caller
 */
static char* echo_body(MessageRef request)
{
    CbufferRef cb_body = Message_serialize(request);
    char* body = malloc(Cbuffer_size(cb_body) + 1);
    memcpy(body, Cbuffer_data(cb_body), Cbuffer_size(cb_body)+1);
    return body;
}
static BufferChainRef echo_body_as_chain(MessageRef request)
{
    CbufferRef cb_body = Message_serialize(request);
    char* body = malloc(Cbuffer_size(cb_body) + 1);
    memcpy(body, Cbuffer_data(cb_body), Cbuffer_size(cb_body)+1);
    BufferChainRef bc = BufferChain_new();
    BufferChain_append(bc, body, Cbuffer_size(cb_body));
    return bc;
}
static IOBufferRef echo_body_iobuffer(MessageRef request)
{
    CbufferRef cb_body = Message_serialize(request);
    int len = Cbuffer_size(cb_body) + 1;
    IOBufferRef iobuf = IOBuffer_new_with_capacity(len);
    void* p = IOBuffer_space(iobuf);
    int iob_len = IOBuffer_space_len(iobuf);
    memcpy(p, Cbuffer_data(cb_body), Cbuffer_size(cb_body));
    IOBuffer_commit(iobuf, iob_len);
    return iobuf;
}
void XrEchoHandler(XrHandlerRef this)
{
    MessageRef request = this->request;
    CbufferRef target = Message_get_target(request);
    char* target_cstr = Cbuffer_cstr(target);
    assert(strcmp(target_cstr, "/echo") == 0);
    HdrListRef req_hdrs = Message_headers(request);
    KVPairRef kvp = HdrList_find(req_hdrs, HEADER_ECHO_ID);
    assert(kvp != NULL);
    HdrListRef resp_hdrs =  HdrList_new();
    HdrList_add_cstr(resp_hdrs, HEADER_ECHO_ID, KVPair_value(kvp));

    IOBufferRef io_body_ref = echo_body_iobuffer (request);
    int body_len = IOBuffer_data_len(io_body_ref);

    char* body_len_str;
    asprintf(&body_len_str, "%d", body_len);

    KVPairRef hl_content_length = KVPair_new(HEADER_CONTENT_LENGTH, strlen(HEADER_CONTENT_LENGTH), body_len_str, strlen(body_len_str));
    HdrList_add_front(resp_hdrs, hl_content_length);
    char* content_type = "text/html; charset=UTF-8";
    KVPairRef hl_content_type = KVPair_new(HEADER_CONTENT_TYPE, strlen(HEADER_CONTENT_TYPE), content_type, strlen(content_type));
    HdrList_add_front(resp_hdrs, hl_content_type);
    HdrList_add_cstr(resp_hdrs, "Connection", "close");
    usleep(10000);
    char* status_line;
    asprintf(&status_line, "HTTP/1.1 %d %s\r\n", 200, "OK");
    IOBufferRef status_line_buffer = IOBuffer_new();
    void* p = IOBuffer_space(status_line_buffer);
    memcpy(p, status_line, strlen(status_line));
    IOBuffer_commit(status_line_buffer, strlen(status_line));

    this->status_line = status_line_buffer;
    this->headers = resp_hdrs;
    this->body = echo_body_as_chain(request);
}
void XrSimpleHandler(XrHandlerRef this)
{
    // make status line
    MessageRef request = this->request;
    char* status_line;
    asprintf(&status_line, "HTTP/1.1 %d %s\r\n", 200, "OK");
    IOBufferRef status_line_buffer = IOBuffer_new();
    void* p = IOBuffer_space(status_line_buffer);
    memcpy(p, status_line, strlen(status_line));
    IOBuffer_commit(status_line_buffer, strlen(status_line));

    CbufferRef body_ref = XrSimpleBody(this, 33);
    int body_len = Cbuffer_size(body_ref);
    char* body_len_str;
    asprintf(&body_len_str, "%d", body_len);

    HdrListRef resp_hdrs =  HdrList_new();

    KVPairRef hl_content_length = KVPair_new(HEADER_CONTENT_LENGTH, strlen(HEADER_CONTENT_LENGTH), body_len_str, strlen(body_len_str));
    HdrList_add_front(resp_hdrs, hl_content_length);
    char* content_type = "text/html; charset=UTF-8";
    KVPairRef hl_content_type = KVPair_new(HEADER_CONTENT_TYPE, strlen(HEADER_CONTENT_TYPE), content_type, strlen(content_type));
    HdrList_add_front(resp_hdrs, hl_content_type);
    HdrList_add_cstr(resp_hdrs, "Connection", "close");

    this->status_line = status_line_buffer;
    this->headers = resp_hdrs;
    this->body = echo_body_as_chain(request);

}
void XrHandlerEngine(XrHandlerRef this)
{
    MessageRef request = this->request;
    char* msg = "<h2>this is a message</h2>";
    char* body = NULL;
    char* body_len_str = NULL;
    HdrListRef resp_hdrs = NULL;
    KVPairRef hl_content_length = NULL;
    KVPairRef hl_content_type = NULL;
    int return_value = 0;
    CbufferRef target = Message_get_target(request);
    char* target_cstr = Cbuffer_cstr(target);
    if(strcmp(target_cstr, "/echo") == 0) {
        XrEchoHandler(this);
    } else {
        XrSimpleHandler(this);
    }
}
XrHandlerRef XrHandler_new(XrConnRef conn_ref)
{
    XrHandlerRef tmp = malloc(sizeof(XrHandler));
    tmp->body = NULL;
    tmp->headers = NULL;
    tmp->conn_ref = conn_ref;             // this is a reference we dont own it
    tmp->request = conn_ref->req_msg_ref; // this is a reference we dont own it
    tmp->status_line = NULL;
    tmp->hdr_iter = NULL;
    tmp->body_iter = NULL;
    return tmp;
}
void XrHandler_free(XrHandlerRef this)
{
    if(this->body != NULL) BufferChain_free(&this->body);
    if(this->headers != NULL) HdrList_free(&this->headers);
    if(this->status_line != NULL) IOBuffer_free(&this->status_line);
}
IOBufferRef XrHandler_status_line(XrHandlerRef this)
{
    return this->status_line;
}
IOBufferRef XrHdrLine(KVPairRef kvp)
{
    char* cstr;
    asprintf(&cstr, "%s: %s\r\n", KVPair_label(kvp), KVPair_value(kvp));
    IOBufferRef buf = IOBuffer_from_cstring(cstr);
    return buf;
}
IOBufferRef XrHandler_header_lines(XrHandlerRef this)
{
    if(this->hdr_iter == NULL) {
        this->hdr_iter = HdrList_iterator(this->headers);
        return XrHdrLine(HdrList_itr_unpack(this->headers, this->hdr_iter));
    }
    this->hdr_iter = HdrList_itr_next(this->headers, this->hdr_iter);
    HdrListIter iter = this->hdr_iter;
    KVPairRef kvp =  HdrList_itr_unpack(this->headers, this->hdr_iter);
    IOBufferRef buf = XrHdrLine(kvp);
    return buf;
}
IOBufferRef XrHandler_body_piece(XrHandlerRef this)
{
    if(this->body_iter == NULL) {
        this->body_iter = BufferChain_iterator(this->body);
        return IOBuffer_from_cbuffer(BufferChain_unpack_iter(this->body, this->hdr_iter));
    }
    this->body_iter = BufferChain_iter_next(this->body, this->body_iter);
    BufferChainIter iter = this->body_iter;
    CbufferRef cbuf =  BufferChain_unpack_iter(this->body, this->body_iter);
    IOBufferRef buf = IOBuffer_from_cbuffer(cbuf);
    return buf;

}
BufferChainRef XrHandler_serialized_response(XrHandlerRef this)
{
    return NULL;
}
