#define _GNU_SOURCE
#include <c_http/async/handler.h>
#include <c_http/logger.h>
#include <c_http/common/message.h>
#include <stdio.h>

void on_done(TcpConnRef conn, XrHandlerRef hdlr);
void on_error(TcpConnRef conn, XrHandlerRef hdlr, int status);
void XrSimpleHandler(XrHandlerRef this);
void XrEchoHandler(XrHandlerRef this);

void XrHandler_function(MessageRef request, TcpConnRef conn_ref, HandlerDoneFunction done)
{
    XrHandlerRef hdlr;
    LOG_FMT("XrHandler_function\n");
    IOBufferRef ser = Message_serialize(conn_ref->req_msg_ref);
    BufferChainRef body = Message_get_body(conn_ref->req_msg_ref);
    if(body != NULL) {
        IOBufferRef cbody = BufferChain_compact(body);
        int blen = BufferChain_size(body);
    }
    // end debugging stuff

    hdlr = XrHandler_new(conn_ref);
    hdlr->state = XRH_STATUS;
    hdlr->done_function = done;
    hdlr->request = request;
    conn_ref->handler_ref = hdlr;
    CbufferRef target = Message_get_target_cbuffer(request);
    const char* target_cstr = Cbuffer_cstr(target);
    if(strcmp(target_cstr, "/echo") == 0) {
        XrEchoHandler(hdlr);
    } else {
        XrSimpleHandler(hdlr);
    }
}
