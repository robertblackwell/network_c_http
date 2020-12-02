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
        return BufferChain_unpack_iter(this->body, this->hdr_iter);
    }
    this->body_iter = BufferChain_iter_next(this->body, this->body_iter);
    BufferChainIter iter = this->body_iter;
    IOBufferRef buf =  BufferChain_unpack_iter(this->body, this->body_iter);
    return buf;
}
BufferChainRef XrHandler_serialized_response(XrHandlerRef this)
{
    return NULL;
}
#ifdef LKLKLKL
IOBufferRef XrHandler_execute(XrHandlerRef hdlr)
{
    assert(hdlr != NULL);
#define XRH_SIMPLE
#ifdef XRH_SIMPLE
    switch(hdlr->state) {
        case XRH_STATUS: {
            XrHandlerEngine(hdlr);

            hdlr->state = XRH_HDRS;
            HdrListRef hdrlist = Message_get_headerlist(hdlr->request);
            hdlr->hdr_iter = HdrList_iterator(hdrlist);
            return hdlr->resp_buf;
        }
        break;
            break;
        case XRH_HDRS:
            return NULL;
            break;
        case XRH_BODY:
            break;
    }

#else
    switch(hdlr->state) {
        case XRH_STATUS: {
            XrHandlerEngine(hdlr);
            hdlr->state = XRH_HDRS;
            HdrListRef hdrlist = Message_headers(hdlr->request);
            hdlr->hdr_iter = HdrList_iterator(hdrlist);
            return hdlr->status_line;
        }
        break;
        case XRH_HDRS: {
            if(hdlr->hdr_iter != NULL) {
                HdrListRef hdrlist = Message_headers(hdlr->request);
                KVPairRef kvp = HdrList_itr_unpack(hdrlist, hdlr->hdr_iter);
                return XrHdrLine(kvp);
            } else {
                hdlr->state = XRH_BODY;
                return XrHandler_execute(hdlr);
            }
        }
        break;
        case XRH_BODY: {
            if(hdlr->body_iter != NULL) {
                IOBufferRef iobuf = XrHandler_body_piece(hdlr);
                return iobuf;
            } else {
                return NULL;
            }
        }
        break;
    }
#endif
}
#endif
