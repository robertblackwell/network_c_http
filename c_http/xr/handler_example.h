#ifndef c_http_xr_handler_example_h
#define c_http_xr_handler_example_h

#include <c_http/message.h>
#include <c_http/xr/conn.h>

struct XrHandler_s;
//struct XrHandler_s {
//    XrConnRef conn_ref;  // weak non owning reference
//    MessageRef      request;   // weak non owning reference
//    IOBufferRef     status_line;
//    HdrListRef      headers;
//    BufferChainRef  body;
//
//    HdrListIter     hdr_iter;
//    BufferChainIter body_iter;
//
//};

IOBufferRef XrHandler_function(MessageRef request, XrConnRef conn);

/**
 * Makes an instance of XrHandler. This instance takes a non owning reference to
 * the XrConn instance and the MessageRef req_msg_ref held by the conn_ref
 * \param conn XrConnRef
 * \return
 */
XrHandlerRef XrHandler_new(XrConnRef conn);
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