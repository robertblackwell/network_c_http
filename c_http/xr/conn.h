#ifndef c_http_xr_conn_h
#define c_http_xr_conn_h
#include <c_http/buffer/iobuffer.h>
#include <c_http/message.h>
#include <c_http/ll_parser.h>
#include <c_http/xr/socket_watcher.h>
#include <c_http/xr/xr_worker.h>

enum XrConnState {
    XRCONN_STATE_UNINIT = 33,
    XRCONN_STATE_READ = 34,
    XRCONN_STATE_HANDLE = 35,
    XRCONN_STATE_WRITE = 36,

};

typedef struct XrConnection_s {
    int                fd;
    enum XrConnState   state;
    XrSocketWatcherRef sock_watcher_ref;
    XrServerRef        server_ref;
    ParserRef          parser_ref;
    IOBufferRef        io_buf_ref;  // input buffer
    MessageRef         req_msg_ref; // request message
    CbufferRef         response_buf_ref; // response as a buffer
} XrConnection, *XrConnectionRef;

XrConnectionRef XrConnection_new(int fd, XrSocketWatcherRef socket_watcher, XrServerRef server_ref);
void XrConnection_free(XrConnectionRef this);

#endif