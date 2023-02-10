
#include <http_in_c/async/handler.h>
#include <http_in_c/logger.h>
#include <http_in_c/common/alloc.h>
#include <http_in_c/http/message.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>


static MessageRef make_simple_response(XrHandlerRef this, MessageRef request);
static void on_write_cb_simple(TcpConnRef conn_ref, void* arg, int status);

static void set_status_ok_200(MessageRef response);
static void set_headers(MessageRef response);
static void set_body_and_content_length_header(MessageRef response, BufferChainRef body);

static BufferChainRef make_simple_body(XrHandlerRef this, int fd);
static void on_done(TcpConnRef conn, XrHandlerRef hdlr);
static void on_error(TcpConnRef conn, XrHandlerRef hdlr, int status);

/**
 * XrSimpleHandler - handles a non echo request
 */
void XrSimpleHandler(XrHandlerRef this)
{
    // make status line
    MessageRef request = this->request;
    MessageRef response = make_simple_response(this, request);
    this->resp_buf = Message_serialize(response);
    TcpConn_write(this->conn_ref, this->resp_buf, on_write_cb_simple, this);
}
/**
 * make_simple_response
 */

static MessageRef make_simple_response(XrHandlerRef this, MessageRef request)
{
    MessageRef response = Message_new_response();
    set_status_ok_200(response);
    set_headers(response);
    int fd = this->conn_ref->fd;
    set_body_and_content_length_header(response, make_simple_body(this, fd));
    return response;
}
static void on_write_cb_simple(TcpConnRef conn_ref, void* arg, int status)
{
    LOG_FMT("conn_ref: %p arg: %p status: %d", conn_ref, arg, status);
    XrHandlerRef hdlr = arg;
    if(status) {
        on_error(conn_ref, hdlr, status);
    } else {
        on_done(conn_ref, hdlr);
    }

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
static BufferChainRef make_simple_body(XrHandlerRef this, int fd)
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
 * on_write_cb on_error on_done
 */
static void on_done(TcpConnRef conn, XrHandlerRef hdlr)
{
    LOG_FMT("on_done\n");
}
static void on_error(TcpConnRef conn, XrHandlerRef hdlr, int status)
{
    LOG_FMT("on_error");
}
