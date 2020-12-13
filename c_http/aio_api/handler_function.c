#define _GNU_SOURCE
#include <c_http/aio_api/handler.h>
#include <c_http/logger.h>
#include <c_http/socket_functions.h>
#include <c_http/api/message.h>

#include <stdio.h>
#include <stdlib.h>

void on_done(XrConnRef conn, XrHandlerRef hdlr);
void on_error(XrConnRef conn, XrHandlerRef hdlr, int status);
void XrSimpleHandler(XrHandlerRef this);
void XrEchoHandler(XrHandlerRef this);

void XrHandler_function(MessageRef request, XrConnRef conn_ref, HandlerDoneFunction done)
{
    XrHandlerRef hdlr;
    LOG_FMT("XrHandler_function\n");
    IOBufferRef ser = Message_serialize(conn_ref->req_msg_ref);
    BufferChainRef body = Message_get_body(conn_ref->req_msg_ref);
    IOBufferRef cbody = BufferChain_compact(body);
    int blen = BufferChain_size(body);
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
