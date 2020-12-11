#ifndef c_http_xr_conn_h
#define c_http_xr_conn_h
#include <c_http/buffer/iobuffer.h>
#include <c_http/message.h>
#include <c_http/ll_parser.h>
#include <c_http/xr/socket_watcher.h>
#include <c_http/xr/handler.h>

#define TYPE XrConn
#define XrConn_TAG "XRCON"
#include <c_http/check_tag.h>
#undef TYPE
#define XR_CONN_DECLARE_TAG DECLARE_TAG(XrConn)
#define XR_CONN_CHECK_TAG(p) CHECK_TAG(XrConn, p)
#define XR_CONN_SET_TAG(p) SET_TAG(XrConn, p)

enum XrConnState {
    XRCONN_STATE_UNINIT = 33,
    XRCONN_STATE_RDINIT = 32, //prepare for read
    XRCONN_STATE_READ = 34,   //read
    XRCONN_STATE_HANDLE = 35,

    XRCONN_STATE_WRITE_STATUS_INIT  = 360,
    XRCONN_STATE_WRITE_STATUS       = 361,
    XRCONN_STATE_WRITE_HDRLINE_INIT = 362,
    XRCONN_STATE_WRITE_HDRLINE      = 363,
    XRCONN_STATE_WRITE_BODY_INIT    = 364,
    XRCONN_STATE_WRITE_BODY         = 365,

    XRCONN_STATE_CLOSE = 37,
    XRCONN_STATE_ERROR = 38,
    XRCONN_STATE_BADREQ = 39,

};
typedef enum XrReadRC {
    XRD_EOM = 0,             // A message was returned
    XRD_EOF = -4,            // Other end closed but no message
    XRD_PERROR = -1,   // An error in the format of the message was detected.
    XRD_ERROR = -2,      // An IO error occurred.
    XRD_EAGAIN = -3
} XrReadRC;


typedef enum XrWriteRC {
    XRW_ERROR = -41,
    XRW_EAGAIN = -42,
    XRW_COMPLETE  = -43
} XrWriteRC;


struct XrConn_s {
    XR_CONN_DECLARE_TAG;
    int                fd;
    enum XrConnState   state;
    XrSocketWatcherRef sock_watcher_ref;
    XrServerRef        server_ref;
    bool               recvbuff_small;

    // read_some variables
    XrSocketWatcherCallback* read_completion_handler; //currently unused

    IOBufferRef              read_some_iobuf;
    XrConnReadCallback*      read_some_cb;
    void*                    read_some_arg;
    int                      bytes_read;
    int                      read_status;

    // read msg variables
    ParserRef                parser_ref;
    IOBufferRef              io_buf_ref;  // input buffer
    MessageRef               req_msg_ref; // request message
    XrConnReadMsgCallback    read_msg_cb;
    void*                    read_msg_arg;
    int                      errno_saved;
    struct ParserError_s     parser_error;

    // writer variables
    CbufferRef               response_buf_ref; // response as a buffer
    IOBufferRef              write_buffer_ref;
//    XrSocketWatcherCallback* write_completion_handler;
    XrConnWriteCallback      write_cb;
    void*                    write_arg;
    XrWriteRC                write_rc;

    XrHandlerRef             handler_ref;
    void*                    handler_ctx;

};
typedef struct XrConn_s XrConn, *XrConnRef;

XrConnRef XrConn_new(int fd, XrSocketWatcherRef socket_watcher, XrServerRef server_ref);
void XrConn_free(XrConnRef this);

void XrConn_read_some(XrConnRef this, IOBufferRef iobuf, XrConnReadCallback cb, void* arg);
void XrConn_read_msg(XrConnRef this, MessageRef msg, XrConnReadMsgCallback cb, void* arg);
void XrConn_write(XrConnRef this, IOBufferRef iobuf, XrConnWriteCallback cb, void* arg);

/**
 * Read a stream of http message from the m_readsocket data source/socket.
 *
 * This Reader object handles the processing of
 * -    taking data from a data source (such as a socket) into a buffers
 * -    pushing the buffer it into a Parser
 * -    handling the action on the various parser output states,
 *      which includes handling the situation where two messages overlap in a single buffer.
 *
 * Note: This function is designed to be used on both blocking and non-blocking byte sources. And hence
 * if the underlying source (socket) has no data available EAGAIN will be returned from the read
 * this will cause Xreader_read to return XRD_EAGAIN.
 * The XrReader_read function should be called again with exactly the same arguments when data is again
 * available on the byte source (socket). The XrReader object retains all required state data
 * to make the repeated call function correctly
 *
 * The reception of a full message is indicated by return of XR_READER_EOM. The message just read will
 * be available in this->message_ref. The XrReader object allocates the Message container.
 *
 * \param this              ReaderRef - the reader object
 * \return Reader_ReturnCode - Indicates whether successfull and if not nature if error.
 *                          TODO - on error the Reader struct will contain details of the error
 *                          for IO error it will hold the errno value related to the error
 *                          and for a parse error will hold the relevant http_errno value
 *                          together with char* pointers to the error name and description
 *
 *  If the reader experiences an io error the erro value will be in this->errno_saved
 *  If the reader experiences a html parsing error and error description will be in this->parser_error
 */
int XrConn_read(XrConnRef this);
/**
 * Called to prepare for a read
 * \param this
 */
void XrConn_prepare_read(XrConnRef this);
void XrConn_prepare_write(XrConnRef this, IOBufferRef buf, XrSocketWatcherCallback completion_handler);
/**
 * Asynchronously writes the provided buffer to this->fd.
 * On completion of the write or error schedules the completion handler to run.
 *
 * The completion handler can assess the outcome of the write operation by examining the write_rc property of
 * the XrConn instance.
 *
 * \param this XrConnRef
 * \param buf  IOBufferRef
 * \param completion_handler XrSocketWatcherCallback
 */
void XrConn_write_2(XrConnRef this, IOBufferRef buf, XrSocketWatcherCallback completion_handler);
/**
 *
 * \param this
 */
void XrConn_done(XrConnRef this);
#endif