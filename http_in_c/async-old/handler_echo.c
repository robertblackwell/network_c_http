
#include <http_in_c/async/handler.h>
#include <rbl/logger.h>
#include <http_in_c/http/message.h>
#include <stdio.h>
#include <stdlib.h>

static void on_done(TcpConnRef conn, XrHandlerRef hdlr);
static void on_error(TcpConnRef conn, XrHandlerRef hdlr, int status);

void XrEchoHandler(XrHandlerRef this);

static MessageRef make_echo_response(MessageRef request);
static void on_write_cb_echo(TcpConnRef conn_ref, void* arg, int status);

static void set_status_ok_200(MessageRef response);
static void set_headers(MessageRef response);
static void set_body_and_content_length_header(MessageRef response, BufferChainRef body);

static IOBufferRef echo_body_iobuffer(MessageRef request);
static BufferChainRef echo_body_as_chain(MessageRef request);
static char* echo_body(MessageRef request);


void XrEchoHandler(XrHandlerRef this)
{
    MessageRef request = this->request;
    MessageRef response = make_echo_response(request);
    this->resp_buf = Message_serialize(response);
    TcpConn_write(this->conn_ref, this->resp_buf, on_write_cb_echo, this);
}
static MessageRef make_echo_response(MessageRef request)
{
    MessageRef response = Message_new_response();
    set_status_ok_200(response);
    set_headers(response);
    set_body_and_content_length_header(response, echo_body_as_chain(request));
    return response;
}
/**
 * set_status_ok, set_headers, set_body_and_content_length
 */
static void set_status_ok_200(MessageRef response)
{
    Message_set_status(response, HTTP_STATUS_OK);
    Message_set_reason(response, "OK");
}
static void set_headers(MessageRef response)
{
    Message_add_header_cstring(response, HEADER_CONTENT_TYPE, "text/html; charset=UTF-8");
    Message_add_header_cstring(response, HEADER_CONNECTION, "close");
}
static void set_body_and_content_length_header(MessageRef response, BufferChainRef body)
{
    int body_length = BufferChain_size(body);
    char* body_len_str;
    asprintf(&body_len_str, "%d", body_length);
    Message_add_header_cstring(response, HEADER_CONTENT_LENGTH, body_len_str);
    free(body_len_str);
    Message_set_body(response, body);
}

static void on_write_cb_echo(TcpConnRef conn_ref, void* arg, int status)
{
    LOG_FMT("conn_ref: %p arg: %p status: %d", conn_ref, arg, status);
    XrHandlerRef hdlr = arg;
    if(status) {
        on_error(conn_ref, hdlr, status);
    } else {
        on_done(conn_ref, hdlr);
    }
}

static void on_done(TcpConnRef conn, XrHandlerRef hdlr)
{
    LOG_FMT("on_done\n");
    hdlr->done_function(conn);
}
static void on_error(TcpConnRef conn, XrHandlerRef hdlr, int status)
{
    LOG_FMT("on_error");
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


