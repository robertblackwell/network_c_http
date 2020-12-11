#include <c_http/xr/conn.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/epoll.h>

XrConnRef XrConn_new(int fd, WSocketRef socket_w, XrServerRef server_ref)
{
    XrConnRef tmp = malloc(sizeof(XrConn));
    XR_CONN_SET_TAG(tmp);
    tmp->fd = fd;
    tmp->sock_watcher_ref = socket_w;
    tmp->server_ref = server_ref;
    tmp->state = XRCONN_STATE_UNINIT;
    tmp->io_buf_ref = IOBuffer_new ();
    tmp->req_msg_ref = Message_new ();
    tmp->response_buf_ref = Cbuffer_new ();
    tmp->parser_ref = Parser_new();
    tmp->recvbuff_small = false;
    tmp->handler_ref = NULL;
    Parser_begin(tmp->parser_ref, tmp->req_msg_ref);
    return tmp;
}

void XrConn_free(XrConnRef this)
{
    XR_CONN_CHECK_TAG(this)
    WSocket_free (this->sock_watcher_ref);
    free(this);
}
#ifdef YES_READ
/*
 * **************************************************************************************************************************
 * read_some
 * **************************************************************************************************************************
 */
static void read_some_handler(WatcherRef watcher, void* arg, uint64_t event);
static void read_some_post_cb(WatcherRef wp, void* arg, uint64_t event)
{
    XrConnRef conn_ref = (XrConnRef)arg;
    (conn_ref->read_some_cb)(conn_ref, conn_ref->read_some_arg, conn_ref->bytes_read, conn_ref->read_status);
}
/**
 * Interface function to read some bytes from a connection. Calls the cb when done.
 * Returns when 1) buffer has been filled 2) return no error when gets EAGAIN error and has red some data
 * 3) gets an io error
 *
 * \param this  XConnRef
 * \param iobuf IOBufferRef
 * \param cb    XrConnReadCallback
 * \param arg   user data
 */
void XrConn_read_some(XrConnRef this, IOBufferRef iobuf, XrConnReadCallback cb, void* arg)
{
    assert(IOBuffer_space_len(iobuf) != 0);
    this->read_some_cb = cb;
    this->read_some_arg = arg;
    if(this->io_buf_ref != NULL) {
        IOBuffer_free(&(this->io_buf_ref));
    }
    this->io_buf_ref = iobuf;
    WSocket_change_watch(this->sock_watcher_ref, &read_some_handler, (void*) this, EPOLLIN | EPOLLERR);
}
/*
 * read_some_handler - called every time fd becomes writeable until the entire IOBuffer is written
 *
 * On completion success or error schedules a calls conn_ref->read_some_cb
 *
 * \param watcher WatcherRef but really WSocketRef.
 * \param arg     void*
 * \param event   uint64_t
 *
 */
static void read_some_handler(WatcherRef wp, void* arg, uint64_t event)
{
    WSocketRef sw = (WSocketRef)wp;
    XrConnRef conn_ref = arg;
    XrReactorRef reactor_ref = sw->runloop;
    IOBufferRef iobuf = conn_ref->io_buf_ref;
    int bytes_read;
    int errno_saved;
    for(;;) {
        void* buf = IOBuffer_space(iobuf);
        int len = IOBuffer_space_len(iobuf);
        bytes_read = recv(conn_ref->fd, buf, len, MSG_DONTWAIT);
        errno_saved = errno;
        conn_ref->bytes_read = bytes_read;
        conn_ref->read_status = 0;
        if(bytes_read == 0) {
            conn_ref->bytes_read = IOBuffer_data_len(conn_ref->io_buf_ref);
            conn_ref->read_status = 0;
            conn_ref->errno_saved = errno_saved;
            WSocket_change_watch(sw, &read_some_post_cb, arg, 0);
            XrReactor_post(reactor_ref, wp, &read_some_post_cb, arg);
            return;
        } else if (bytes_read < 0) {
            if (errno_saved == EAGAIN) {
                conn_ref->bytes_read = IOBuffer_data_len(conn_ref->io_buf_ref);
                conn_ref->read_status = 0;
            } else {
                conn_ref->errno_saved = errno_saved;
                conn_ref->read_status = errno_saved;
            }
            WSocket_change_watch(sw, &read_some_post_cb, arg, 0);
            XrReactor_post(reactor_ref, wp, &read_some_post_cb, arg);
            return;
        } else /* (bytes_read > 0) */{
            IOBuffer_commit(iobuf, bytes_read);
        }
    }
}
#endif
#ifdef YES_WRITE
/*
 * **************************************************************************************************************************
 * XrConn_write
 * **************************************************************************************************************************
 */
static void write_event_handler(WatcherRef wp, void* arg, uint64_t event)
{
    WSocketRef sw = (WSocketRef)wp;
    XrConnRef conn_ref = arg;
    XrReactorRef reactor_ref = sw->runloop;
    XR_TRACE("conn_ref: %p", conn_ref);
    void* p = IOBuffer_data(conn_ref->write_buffer_ref);
    int len = IOBuffer_data_len(conn_ref->write_buffer_ref);
    int bytes_written = send(conn_ref->fd, p, len, MSG_DONTWAIT);
    if(bytes_written > 0) {
        IOBuffer_consume(conn_ref->write_buffer_ref, bytes_written);
        int t = IOBuffer_data_len(conn_ref->write_buffer_ref);
    } else if(bytes_written == 0) {
    } else {
    }
    assert(conn_ref->handler_ref != NULL);
    WSocket_change_watch(sw, &write_event_handler, arg, 0);
    XrReactor_post(reactor_ref, wp, conn_ref->write_cb, conn_ref);

}
void XrConn_write(XrConnRef this, IOBufferRef iobuf, XrConnWriteCallback cb, void* arg)
{
    this->write_buffer_ref = iobuf;
    this->write_arg = arg;
    this->write_cb = cb;
    WSocketRef sw = this->sock_watcher_ref;
    WSocket_change_watch(sw, &write_event_handler, arg, EPOLLOUT|EPOLLERR);
}
#endif
#ifdef YES_READ
/*
 * **************************************************************************************************************************
 * XrConn_read_msg
 * **************************************************************************************************************************
 */
static void read_msg_init(WatcherRef wp, void *arg, uint64_t event);
static void read_msg_handler(WatcherRef wp, void *arg, uint64_t event);
void XrConn_prepare_read(XrConnRef this);

void XrConn_read_msg(XrConnRef this, MessageRef msg, XrConnReadMsgCallback cb, void* arg)
{
    this->read_msg_cb = cb;
    this->read_msg_arg = arg;
    this->req_msg_ref = msg;
    XrConn_prepare_read(this);
    WSocketRef sw = this->sock_watcher_ref;
    XrReactorRef reactor_ref = sw->runloop;
    uint64_t interest = EPOLLERR | EPOLLIN;
    WSocket_register(sw, &read_msg_handler, this, interest);

}

// XrConn_read Reads a message with repeated calls and returns status after each call
static void free_req_message(XrConnRef this)
{
    if(this->req_msg_ref != NULL) {
        Message_free(&(this->req_msg_ref));
    }
}
void XrConn_prepare_read(XrConnRef this)
{
    if(this->recvbuff_small) {
        // this is a trick to make EAGAIN errors happen to test the handler state machine and the reader function
        int recvbufflen_out;
        int optlen = sizeof(recvbufflen_out);
        int r = getsockopt(this->fd, SOL_SOCKET, SO_RCVBUF, &recvbufflen_out, &optlen);
        int recvbuflen = 1024;
        r = setsockopt(this->fd, SOL_SOCKET, SO_RCVBUF, &recvbuflen, optlen);
        r = getsockopt(this->fd, SOL_SOCKET, SO_RCVBUF, &recvbufflen_out, &optlen);
    }
    if(this->req_msg_ref == NULL) {
        this->req_msg_ref = Message_new();
    }
    Parser_begin(this->parser_ref, this->req_msg_ref);
}
/**
 * A function that can be XrReactor_post()'d that will call the read_msg_cb
 * with the correct parameters
 * \param wp    WatcherRef
 * \param arg   void*
 * \param event uint64_t
 */
static void read_msg_cb_wrapper(WatcherRef wp, void *arg, uint64_t event)
{
    WSocketRef sw = (WSocketRef)wp;
    XrConnRef conn_ref = arg;
    XrReactorRef reactor_ref = sw->runloop;
    conn_ref->read_msg_cb(conn_ref, arg, conn_ref->read_status);
}
/**
 * Handles data available events when reading a full message
 * On completion posts the read_mesg_cb
 * \param wp  WatcherRef but really WSocketRef
 * \param arg void* use data
 * \param event uint64_t
 */
static void read_msg_handler(WatcherRef wp, void *arg, uint64_t event)
{
    WSocketRef sw = (WSocketRef)wp;
    XrConnRef conn_ref = arg;
    XrReactorRef reactor_ref = sw->runloop;

#define NEXT_STATE(state) \
    conn_ref->state = state; \
    WSocket_change_watch(sw, &state_machine, arg, 0); \
    XrReactor_post(reactor_ref, sw, &state_machine, arg);

    printf("XrWorker::wrkr_state_machine fd: %d\n", conn_ref->fd);
    uint64_t e1 = EPOLLIN;
    uint64_t e2 = EPOLLOUT;
    uint64_t e3 = EPOLLERR;
    uint64_t e4 = EPOLLHUP;
    uint64_t e5 = EPOLLRDHUP;
    bool pollin = (event & EPOLLIN);
    bool pollout = (event & EPOLLOUT);
    bool pollerr = (event & EPOLLERR);
    bool pollhup = (event & EPOLLHUP);
    bool pollrdhup = (event & EPOLLRDHUP);
    assert(conn_ref->req_msg_ref != NULL);
    assert(conn_ref->parser_ref != NULL);
    XrReadRC rc = XrConn_read(conn_ref);
    // have to decide what next
    XR_PRINTF("XrServer::state_machine after read rc :%d \n", rc);
    if (rc == XRD_EAGAIN) {
        XR_PRINTF("XrServer::state_machine EAGAIN\n");
        return;
    } else {
        if(rc == XRD_EOM) {
            XR_PRINTF("XrServer::state_machine EOM\n");
            assert(conn_ref->parser_ref->m_message_done);
            assert(conn_ref->req_msg_ref != NULL);
            assert(conn_ref->req_msg_ref == conn_ref->parser_ref->m_current_message_ptr);
            conn_ref->read_status = XRD_EOM;
        } else if(rc == XRD_EOF) {
            XR_PRINTF("XrServer::state_machine EOF\n");
            conn_ref->read_status = rc;
        } else if(rc == XRD_ERROR) {
            XR_PRINTF("XrServer::state_machine XRD_ERROR\n");
            conn_ref->read_status = rc;
        } else {
            XR_PRINTF("XrServer::state_machine XRD_PERROR\n");
            assert(rc == XRD_PERROR);
            conn_ref->read_status = XRD_PERROR;
        }
        WSocket_change_watch(sw, &read_msg_handler, arg, 0);
        XrReactor_post(reactor_ref, wp, &read_msg_cb_wrapper, arg);
        return;
    }
}
static void read_msg_init(WatcherRef wp, void *arg, uint64_t event)
{
    WSocketRef sw = (WSocketRef)wp;
    XrConnRef conn_ref = arg;
    XrReactorRef reactor_ref = sw->runloop;
    uint64_t interest = EPOLLERR | EPOLLIN;
    WSocket_register(sw, &read_msg_handler, conn_ref, interest);
}
int XrConn_read(XrConnRef this)
{
    IOBufferRef iobuf = this->io_buf_ref;
    int bytes_read;
    int errno_saved;
    for(;;) {
        //
        // handle nothing left in iobuffer
        // only read more if iobuffer is empty
        //
        if(IOBuffer_data_len(iobuf) == 0 ) {
            IOBuffer_reset(iobuf);
            void* buf = IOBuffer_space(iobuf);
            int len = IOBuffer_space_len(iobuf);
            bytes_read = recv(this->fd, buf, len, MSG_DONTWAIT);
            errno_saved = errno;
            if(bytes_read == 0) {
                if (! this->parser_ref->m_started) {
                    // eof no message started - there will not be any more bytes to parse so cleanup and return eof
                    free_req_message(this);
                    return XRD_EOF;
                }
                if (this->parser_ref->m_started && this->parser_ref->m_message_done) {
                    // should not get here
                    assert(false);
                }
                if (this->parser_ref->m_started && !this->parser_ref->m_message_done) {
                    // get here if other end is signlaling eom with eof
                    assert(bytes_read == 0);
                    assert(true);
                }
            } else if (bytes_read < 0) {
                if (errno_saved == EAGAIN) {
                    return XRD_EAGAIN;
                } else {
                    // have an io error
                    this->errno_saved = errno_saved;
                    free_req_message(this);
                    return XRD_ERROR;
                }
            } else /* (bytes_read > 0) */{
                IOBuffer_commit(iobuf, bytes_read);
            }
        } else {
            bytes_read = iobuf->buffer_remaining;
        }
        char* tmp = (char*)iobuf->buffer_ptr;
        char* tmp2 = (char*)iobuf->mem_p;
        ParserReturnValue ret = Parser_consume(this->parser_ref, (void*) IOBuffer_data(iobuf), IOBuffer_data_len(iobuf));
        int consumed = bytes_read - ret.bytes_remaining;
        IOBuffer_consume(iobuf, consumed);
        int tmp_remaining = iobuf->buffer_remaining;
        enum ParserRC rc = ret.return_code;
        if(rc == ParserRC_end_of_data) {
            ;  // ok end to Parser_consume call - get more data
        } else if(rc == ParserRC_end_of_header) {
            ;  // ok end to Parser_consume call - get more data - in future return if reading only headers
        } else if(rc == ParserRC_end_of_message) {
            return XRD_EOM;
        } else if(rc == ParserRC_error) {
            this->parser_error = Parser_get_error(this->parser_ref);
            free_req_message(this);
            return XRD_PERROR;
        }

    }
}
#endif
#ifdef YES_WRITE

void XrConn_prepare_write_2(XrConnRef this, IOBufferRef buf, SocketEventHandler completion_handler)
{
}
/**
 * This function is called by the Reactor on EPOLLOUT or EPOLLERR. It keeps getting called until
 * the entire buffer is written or an error occurs which is not an EAGAIN
 * \param wp    WatcherRef (but really an WSocketRef)
 * \param arg   void* But typecast it to XrConnRef
 * \param event EPOLL event not used
 *
 * Returns result code XrWriteRC in XrConnRef->write_rc
 * and uses XrConnRef->write_???? properties as saved context between
 * invocations.
 *
 */
void write_state_machine(WatcherRef wp, void* arg, uint64_t event)
{
    WSocketRef sw = (WSocketRef)wp;
    XrConnRef conn_ref = (XrConnRef)arg;
    XrReactorRef reactor_ref = sw->runloop;
    // dont accept empty buffers
    assert(IOBuffer_data_len(conn_ref->write_buffer_ref) != 0);

    for(;;) {
        void* bptr = IOBuffer_data(conn_ref->write_buffer_ref);
        int blen = IOBuffer_data_len(conn_ref->write_buffer_ref);

        int nwrite = send(conn_ref->fd, bptr, blen, MSG_DONTWAIT);
        int saved_errno = errno;
        assert(nwrite != 0);

        if(nwrite > 0) {
            IOBuffer_consume(conn_ref->write_buffer_ref, nwrite);
            blen = IOBuffer_data_len(conn_ref->write_buffer_ref);
            if(blen > 0) {
                continue;
            } else {
                conn_ref->write_rc = XRW_COMPLETE;
                WSocket_change_watch(sw, conn_ref->write_completion_handler, arg, 0);
                XrReactor_post(reactor_ref, wp, conn_ref->write_completion_handler, arg);
                break;
            }
        } else {
            assert(nwrite < 0);
            if((saved_errno == EAGAIN) || (saved_errno == EWOULDBLOCK)) {
                conn_ref->write_rc = XRW_EAGAIN;
                return; // wait for the next write ready event, which will call this function to complete the buffer
            } else {
                conn_ref->write_rc = XRW_ERROR;
                WSocket_change_watch(sw, conn_ref->write_completion_handler, arg, 0);
                XrReactor_post(reactor_ref, wp, conn_ref->write_completion_handler, arg);
                break;
            }
        }
    }
}
void XrConn_write_2(XrConnRef this, IOBufferRef buf, SocketEventHandler completion_handler)
{
    this->write_buffer_ref = buf;
    this->write_completion_handler = completion_handler;
    WSocketRef sw = this->sock_watcher_ref;
    WSocket_change_watch(sw, &write_state_machine, (void*)this, EPOLLERR | EPOLLOUT);
}
#endif