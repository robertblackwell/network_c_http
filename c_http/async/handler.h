#ifndef c_http_xr_handler_example_h
#define c_http_xr_handler_example_h

#include <c_http/common/message.h>
#include <c_http/common/hdrlist.h>
#include <c_http/async/types.h>
#include <c_http/async/tcp_conn.h>
#include "types.h"
#include "tcp_conn.h"
#include "tcp_conn_list.h"

struct XrHandler_s;
typedef enum XrHandlerState {
    XRH_INIT = 90,
    XRH_STATUS = 91,
    XRH_HDRS = 92,
    XRH_BODY = 93,
} XrHandlerState;

struct XrHandler_s {
    TcpConnRef conn_ref;  // weak non owning reference
    MessageRef      request;   // weak non owning reference
    IOBufferRef     status_line;
    HdrListRef      headers;
    BufferChainRef  body;
    MessageRef      response;
    IOBufferRef     resp_buf;
    HdrListIter     hdr_iter;
    BufferChainIter body_iter;
    XrHandlerState  state;
    HandlerDoneFunction done_function;
};

/**
 * This is an example of a handler function to be passed to an instance of xr_server
 * @param request
 * @param conn
 * @param done
 * @return nothing
 */
void XrHandler_function(MessageRef request, TcpConnRef conn, HandlerDoneFunction done);

/**
 * Makes an instance of XrHandler. This instance takes a non owning reference to
 * the TcpConn instance and the MessageRef req_msg_ref held by the conn_ref
 * \param conn TcpConnRef
 * \return
 */
XrHandlerRef XrHandler_new(TcpConnRef conn);
void XrHandler_free(XrHandlerRef this);
/**
 * Returns an IOBufferRef containing the status line of the response
 * \param this XrHandlerRef
 * \return IOBUfferRef
 */
IOBufferRef XrHandler_status_line(XrHandlerRef this);

/**
 * Returns an IOBufferRef containing a single header line or NULL
 * NULL signals all header lines have been returned.
 * \param this XrHandlerRef
 * \return IOBUfferRef
 */
IOBufferRef XrHandler_header_lines(XrHandlerRef this);
/**
 * Returns a sequence of IOBUfferRef which contain successive piece of the response body.
 * Returns NULL when the sequence is complete.
 * NOTE: currently NOT chunk encoded so body must be known before headers are written
 *
 * \param this XrHandlerRef
 * \return IOBufferRef
 */
IOBufferRef XrHandler_body_piece(XrHandlerRef this);
/**
 * Returns the entire response message as a BUfferChain
 * \param this XrHandlerRef
 * \return BufferChainRef
 */
BufferChainRef XrHandler_serialized_response(XrHandlerRef this);

IOBufferRef XrHandler_execute(XrHandlerRef this);

#endif