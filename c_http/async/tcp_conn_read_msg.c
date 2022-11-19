#include <c_http//simple_runloop/runloop.h>
#include <c_http/simple_runloop/rl_internal.h>
#include <c_http/async/tcp_conn.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <c_http/logger.h>
///*
// * **************************************************************************************************************************
// * read_some
// * **************************************************************************************************************************
// */
//static void read_some_handler(RtorStreamRef watcher, void* arg, uint64_t event);
//static void read_some_post_func(void* arg)
//{
//    TcpConnRef conn_ref = (TcpConnRef)arg;
//    TCP_CONN_CHECK_TAG(conn_ref)
//    (conn_ref->read_some_cb)(conn_ref, conn_ref->read_some_arg, conn_ref->bytes_read, conn_ref->read_status);
//}
///**
// * Interface function to read some bytes from a connection. Calls the cb when done.
// * Returns when 1) buffer has been filled 2) return no error when gets EAGAIN error and has red some data
// * 3) gets an io error
// *
// * \param this  XConnRef
// * \param iobuf IOBufferRef
// * \param cb    TcpConnReadCallback
// * \param arg   user data
// */
//void TcpConn_read_some(TcpConnRef this, IOBufferRef iobuf, TcpConnReadCallback cb, void* arg)
//{
//    TCP_CONN_CHECK_TAG(this)
//    assert(IOBuffer_space_len(iobuf) != 0);
//    this->read_some_cb = cb;
//    this->read_some_arg = arg;
//    if(this->io_buf_ref != NULL) {
//        IOBuffer_dispose(&(this->io_buf_ref));
//    }
//    this->io_buf_ref = iobuf;
//    rtor_stream_arm_read(this->sock_watcher_ref, &read_some_handler, (void*) this);
//}
///*
// * read_some_handler - called every time fd becomes writeable until the entire IOBuffer is written
// *
// * On completion success or error schedules a calls conn_ref->read_some_cb
// *
// * \param watcher RtorWatcherRef but really RtorStreamRef.
// * \param arg     void*
// * \param event   uint64_t
// *
// */
//static void read_some_handler(RtorStreamRef socket_watcher_ref, void* arg, uint64_t event)
//{
//    TcpConnRef conn_ref = arg;
//    TCP_CONN_CHECK_TAG(conn_ref)
//
//    ReactorRef reactor_ref = socket_watcher_ref->simple_runloop;
//    IOBufferRef iobuf = conn_ref->io_buf_ref;
//    int bytes_read;
//    int errno_saved;
//    for(;;) {
//        void* buf = IOBuffer_space(iobuf);
//        int len = IOBuffer_space_len(iobuf);
//        bytes_read = recv(conn_ref->fd, buf, len, MSG_DONTWAIT);
//        errno_saved = errno;
//        conn_ref->bytes_read = bytes_read;
//        conn_ref->read_status = 0;
//        if(bytes_read == 0) {
//            conn_ref->bytes_read = IOBuffer_data_len(conn_ref->io_buf_ref);
//            conn_ref->read_status = 0;
//            conn_ref->errno_saved = errno_saved;
//            // @TODO - fix next 2 lines
////            WIoFd_change_watch(sw, &read_some_post_cb, arg, 0);
////            WIoFd_disarm(sw, XR_READ);
//            rtor_reactor_post(reactor_ref, &read_some_post_func, conn_ref);
//            return;
//        } else if (bytes_read < 0) {
//            if (errno_saved == EAGAIN) {
//                conn_ref->bytes_read = IOBuffer_data_len(conn_ref->io_buf_ref);
//                conn_ref->read_status = 0;
//            } else {
//                conn_ref->errno_saved = errno_saved;
//                conn_ref->read_status = errno_saved;
//            }
//            // @TODO - fix next 2 lines
////            WIoFd_change_watch(sw, &read_some_post_cb, arg, 0);
////            WIoFd_disarm(sw, XR_READ);
//            rtor_reactor_post(reactor_ref, &read_some_post_func, conn_ref);
//            return;
//        } else /* (bytes_read > 0) */{
//            IOBuffer_commit(iobuf, bytes_read);
//        }
//    }
//}

/*
 * **************************************************************************************************************************
 * TcpConn_read_msg
 * **************************************************************************************************************************
 */
//static void read_msg_init(RtorWatcherRef wp, void *arg, uint64_t event);
static void read_msg_handler(RtorStreamRef wp, uint64_t event);
void TcpConn_prepare_read(TcpConnRef this);

void TcpConn_read_msg(TcpConnRef this, MessageRef msg, TcpConnReadMsgCallback cb, void* arg)
{
    TCP_CONN_CHECK_TAG(this)
    this->read_msg_cb = cb;
    this->read_msg_arg = arg;
    this->req_msg_ref = msg;
    TcpConn_prepare_read(this);
    RtorStreamRef sw = this->sock_watcher_ref;
    ReactorRef reactor_ref = rtor_stream_get_reactor(sw);
    uint64_t interest = EPOLLERR | EPOLLIN;
    rtor_stream_register(sw);
    rtor_stream_arm_read(sw, read_msg_handler, arg);
}

// TcpConn_read Reads a message with repeated calls and returns status after each call
static void free_req_message(TcpConnRef this)
{
    TCP_CONN_CHECK_TAG(this)
    if(this->req_msg_ref != NULL) {
        Message_dispose(&(this->req_msg_ref));
    }
}
void TcpConn_prepare_read(TcpConnRef this)
{
    TCP_CONN_CHECK_TAG(this)
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
 * A function that can be rtor_reactor_post()'d that will call the read_msg_cb
 * with the correct parameters
 * \param wp    RtorWatcherRef
 * \param arg   void*
 * \param event uint64_t
 */
static void on_post_read_msg(ReactorRef rtor_ref, void *arg)
{
    TcpConnRef conn_ref = arg;
    RtorStreamRef sw = conn_ref->sock_watcher_ref;
    TCP_CONN_CHECK_TAG(conn_ref)
    ReactorRef reactor_ref = rtor_stream_get_reactor(sw);
    conn_ref->read_msg_cb(conn_ref, arg, conn_ref->read_status);
}
/**
 * Handles data available events when reading a full message
 * On completion posts the read_mesg_cb
 * \param wp  RtorWatcherRef but really RtorStreamRef
 * \param arg void* use data
 * \param event uint64_t
 */
static void read_msg_handler(RtorStreamRef socket_watcher_ref, uint64_t event)
{
    void* arg = socket_watcher_ref->read_arg;
    RtorStreamRef sw = socket_watcher_ref;
    TcpConnRef conn_ref = arg;
    TCP_CONN_CHECK_TAG(conn_ref)
    ReactorRef reactor_ref = rtor_stream_get_reactor(sw);

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
    XrReadRC rc = TcpConn_read(conn_ref);
    // have to decide what next
    LOG_FMT("AsyncServer::state_machine after read rc :%d \n", rc);
    if (rc == XRD_EAGAIN) {
        LOG_FMT("AsyncServer::state_machine EAGAIN\n");
        return;
    } else {
        if(rc == XRD_EOM) {
            LOG_FMT("AsyncServer::state_machine EOM\n");
            assert(conn_ref->parser_ref->m_message_done);
            assert(conn_ref->req_msg_ref != NULL);
            assert(conn_ref->req_msg_ref == conn_ref->parser_ref->m_current_message_ptr);
            conn_ref->read_status = XRD_EOM;
        } else if(rc == XRD_EOF) {
            LOG_FMT("AsyncServer::state_machine EOF\n");
            conn_ref->read_status = rc;
        } else if(rc == XRD_ERROR) {
            LOG_FMT("AsyncServer::state_machine XRD_ERROR\n");
            conn_ref->read_status = rc;
        } else {
            LOG_FMT("AsyncServer::state_machine XRD_PERROR\n");
            assert(rc == XRD_PERROR);
            conn_ref->read_status = XRD_PERROR;
        }
        // @TODO fix next 2 lines
//        WIoFd_change_watch(sw, &read_msg_handler, arg, 0);
        rtor_stream_disarm_read(sw);
        rtor_reactor_post(reactor_ref, &on_post_read_msg, conn_ref);
        return;
    }
}
//static void read_msg_init(RtorWatcherRef wp, void *arg, uint64_t event)
//{
//    RtorStreamRef sw = (RtorStreamRef)wp;
//    TcpConnRef conn_ref = arg;
//    ReactorRef reactor_ref = sw->simple_runloop;
//    uint64_t interest = EPOLLERR | EPOLLIN;
//    rtor_stream_register(sw, &read_msg_handler, conn_ref, interest);
//}
int TcpConn_read(TcpConnRef this)
{
    TCP_CONN_CHECK_TAG(this)
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
            bytes_read = IOBuffer_data_len(iobuf);
        }
        char* tmp = IOBuffer_data(iobuf);
        char* tmp2 = IOBuffer_memptr(iobuf);
        ParserReturnValue ret = Parser_consume(this->parser_ref, (void*) IOBuffer_data(iobuf), IOBuffer_data_len(iobuf));
        int consumed = bytes_read - ret.bytes_remaining;
        IOBuffer_consume(iobuf, consumed);
        int tmp_remaining = IOBuffer_data_len(iobuf);
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
