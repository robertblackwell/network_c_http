#define _GNU_SOURCE
#include <c_http/aio_api/handler.h>

#include <c_http/constants.h>
#include <c_http/dsl/alloc.h>
#include <c_http/dsl/utils.h>
#include <c_http/socket_functions.h>
#include <c_http/api/message.h>
#include <c_http/details/ll_parser.h>

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
