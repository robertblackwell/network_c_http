#define _GNU_SOURCE
#include <c_http/xr/handler.h>

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

void on_done(XrConnRef conn, XrHandlerRef hdlr);
void on_error(XrConnRef conn, XrHandlerRef hdlr, int status);
static BufferChainRef XrSimpleBody(XrHandlerRef this, int fd);
void XrSimpleHandler(XrHandlerRef this);
void XrEchoHandler(XrHandlerRef this);
void XrHandlerEngine(XrHandlerRef this);
static void on_write_cb_simple(XrConnRef conn_ref, void* arg, int status);
static IOBufferRef echo_body_iobuffer(MessageRef request);
static BufferChainRef echo_body_as_chain(MessageRef request);
static char* echo_body(MessageRef request);

void XrHandler_function(MessageRef request, XrConnRef conn_ref, HandlerDoneFunction done)
{
    XrHandlerRef hdlr;
    XR_PRINTF("XrHandler_function\n");
    if(conn_ref->handler_ref == NULL) {
        // this is debugging stuff
        IOBufferRef ser = Message_serialize(conn_ref->req_msg_ref);
        BufferChainRef body = Message_get_body(conn_ref->req_msg_ref);
        IOBufferRef cbody = BufferChain_compact(body);
        int blen = BufferChain_size(body);
        // end debugging stuff
        hdlr = XrHandler_new(conn_ref);
        hdlr->state = XRH_STATUS;
        hdlr->done_function = done;
        conn_ref->handler_ref = hdlr;
    } else {
        hdlr = conn_ref->handler_ref;
    }
    XrHandlerEngine(hdlr);
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
    CbufferRef target = Message_get_target_cbuffer(request);
    const char* target_cstr = Cbuffer_cstr(target);
    if(strcmp(target_cstr, "/echo") == 0) {
        XrEchoHandler(this);
    } else {
        XrSimpleHandler(this);
    }
}

void XrEchoHandler(XrHandlerRef this)
{
    MessageRef request = this->request;
    this->response = Message_new_response();

    CbufferRef target = Message_get_target_cbuffer(request);
    const char* target_cstr = Cbuffer_cstr(target);
    assert(strcmp(target_cstr, "/echo") == 0);
    HdrListRef req_hdrs = Message_get_headerlist(request);
//    KVPairRef kvp = HdrList_find(req_hdrs, HEADER_ECHO_ID);
//    assert(kvp != NULL);
    HdrListRef resp_hdrs =  Message_get_headerlist(this->response);
//    HdrList_add_cstr(resp_hdrs, HEADER_ECHO_ID, KVPair_value(kvp));
    Message_set_status(this->response, HTTP_STATUS_OK);
    CbufferRef cb_reason = Cbuffer_from_cstring("OK");
    Message_set_reason_cbuffer(this->response, cb_reason);
    Cbuffer_free(&cb_reason);
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
    Message_set_body(this->response, this->body);
    IOBufferRef tmpb = Message_serialize(this->response);
    this->resp_buf = tmpb;
    XR_TRACE("resp_buf len %d ", IOBuffer_data_len(this->resp_buf));
    XrConn_write(this->conn_ref, this->resp_buf, on_write_cb_simple, this);

}
void XrSimpleHandler(XrHandlerRef this)
{
    // make status line
    MessageRef request = this->request;
    this->response = Message_new_response();
    Message_set_status(this->response, HTTP_STATUS_OK);
    CbufferRef reason = Cbuffer_from_cstring("OK");
    Message_set_reason_cbuffer(this->response, reason);
    HdrListRef resp_hdrs = Message_get_headerlist(this->response);
    char* status_line;
    asprintf(&status_line, "HTTP/1.1 %d %s\r\n", 200, "OK");
    IOBufferRef status_line_buffer = IOBuffer_new();
    void* p = IOBuffer_space(status_line_buffer);
    memcpy(p, status_line, strlen(status_line));
    IOBuffer_commit(status_line_buffer, strlen(status_line));

    BufferChainRef body_ref = XrSimpleBody(this, 33);
    int body_len = BufferChain_size(body_ref);
    char* body_len_str;
    asprintf(&body_len_str, "%d", body_len);

    KVPairRef hl_content_length = KVPair_new(HEADER_CONTENT_LENGTH, strlen(HEADER_CONTENT_LENGTH), body_len_str, strlen(body_len_str));
    HdrList_add_front(resp_hdrs, hl_content_length);
    char* content_type = "text/html; charset=UTF-8";
    KVPairRef hl_content_type = KVPair_new(HEADER_CONTENT_TYPE, strlen(HEADER_CONTENT_TYPE), content_type, strlen(content_type));
    HdrList_add_front(resp_hdrs, hl_content_type);
    HdrList_add_cstr(resp_hdrs, "Connection", "close");

    this->status_line = status_line_buffer;
    this->headers = resp_hdrs;
    this->body = body_ref;
    Message_set_body(this->response, this->body);
    IOBufferRef tmpb = Message_serialize(this->response);
    this->resp_buf = tmpb;
    XR_TRACE("resp_buf len %d ", IOBuffer_data_len(this->resp_buf));
    XrConn_write(this->conn_ref, this->resp_buf, on_write_cb_simple, this);
}

static void on_write_cb_simple(XrConnRef conn_ref, void* arg, int status)
{
    XR_TRACE("conn_ref: %p arg: %p status: %d", conn_ref, arg, status);
    XrHandlerRef hdlr = arg;
    if(status) {
        on_error(conn_ref, hdlr, status);
    } else {
        on_done(conn_ref, hdlr);
    }

}

void on_done(XrConnRef conn, XrHandlerRef hdlr)
{
    XR_PRINTF("on_done\n");
}
void on_error(XrConnRef conn, XrHandlerRef hdlr, int status)
{
    XR_PRINTF("on_error");
}
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
static BufferChainRef XrSimpleBody(XrHandlerRef this, int fd)
{
    IOBufferRef serialized_request = Message_serialize(this->request);

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char date_time_str[64];
    assert(strftime(date_time_str, sizeof(date_time_str), "%c", tm));

    char* body_fmt = "<html>"
                 "<head>"
                 "</head>"
                 "<body>"
                 "%.*s"   // format for string without null terminator must provide len, ptr in that order in the arg list
                 "<p>Date/Time is %s</p>"
                 "<p>socket: %d</p>"
                 "<p>p_thread_self %ld</p>"
                 "</body>"
                 "</html>";

    char* s1;
    int len1 = asprintf(&s1, body_fmt, IOBuffer_data_len(serialized_request), IOBuffer_data(serialized_request), date_time_str, fd);
    BufferChainRef bc = BufferChain_new();
    BufferChain_append_cstr(bc, s1);
    free(s1);
    return bc;
}

/**
 * Create the body of a response to an /echo request
 * \param request
 * \return char* ownership, and responsibility to free, transfers to the caller
 */
static char* echo_body(MessageRef request)
{
    IOBufferRef iob_body = Message_serialize(request);
    char* body = malloc(IOBuffer_data_len(iob_body) + 1);
    memcpy(body, IOBuffer_data(iob_body), IOBuffer_data_len(iob_body)+1);
    return body;
}
static BufferChainRef echo_body_as_chain(MessageRef request)
{
    IOBufferRef iob_body = Message_serialize(request);
    char* body = malloc(IOBuffer_data_len(iob_body) + 1);
    memcpy(body, IOBuffer_data(iob_body), IOBuffer_data_len(iob_body)+1);
    BufferChainRef bc = BufferChain_new();
    BufferChain_append_IOBuffer(bc, iob_body);
    return bc;
}
static IOBufferRef echo_body_iobuffer(MessageRef request)
{
    IOBufferRef iob_body = Message_serialize(request);
    return iob_body;
}


