#include <c_http/aio_api/conn.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <c_http/logger.h>

/*
 * **************************************************************************************************************************
 * XrConn_write
 * **************************************************************************************************************************
 */
/**
 * when a write operation is complete (no ready or written all data) and going to
 * call completion function .. this function gets posted to the fun list
 * Its job is to call the completion cb
 */
static void write_complete_post_func(void* arg)
{
    XrConnRef conn_ref = arg;
    XR_CONN_CHECK_TAG(conn_ref)
    LOG_FMT("conn_ref: %p arg: %p ", conn_ref, arg);
    conn_ref->write_cb(conn_ref, conn_ref->write_arg, 0);
}

static void write_event_handler(WatcherRef wp, void* arg, uint64_t event)
{
    WSocketRef sw = (WSocketRef)wp;
    XrConnRef conn_ref = arg;
    XR_CONN_CHECK_TAG(conn_ref)
    XrReactorRef reactor_ref = sw->runloop;
    LOG_FMT("conn_ref: %p", conn_ref);
    void* p = IOBuffer_data(conn_ref->write_buffer_ref);
    int len = IOBuffer_data_len(conn_ref->write_buffer_ref);
    int bytes_written = send(conn_ref->fd, p, len, MSG_DONTWAIT);
    int errno_saved = errno;
    if(bytes_written > 0) {

        IOBuffer_consume(conn_ref->write_buffer_ref, bytes_written);
        int t = IOBuffer_data_len(conn_ref->write_buffer_ref);
    } else if(bytes_written == 0) {
    } else {
    }
    assert(conn_ref->handler_ref != NULL);
    // @TODO fix next 2 lines
//    WSocket_change_watch(sw, &write_event_handler, arg, 0);
    WSocket_disarm_write(sw);
    XrReactor_post(reactor_ref, write_complete_post_func, conn_ref);

}
void write_machine(WSocketRef socket_watcher_ref, void* arg, uint64_t event);
void XrConn_write(XrConnRef this, IOBufferRef iobuf, XrConnWriteCallback cb, void* arg)
{
    LOG_FMT("conn_ref: %p iobuf: %p iobuf len: %d arg: %p", this, iobuf, IOBuffer_data_len(iobuf), arg);
    XR_CONN_CHECK_TAG(this)
    this->write_buffer_ref = iobuf;
    this->write_arg = arg;
    this->write_cb = cb;
//    this->write_completion_handler = &write_complete_post_func;
    WSocketRef sw = this->sock_watcher_ref;
    WSocket_arm_write(sw, &write_machine, this);
}

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
void write_machine(WSocketRef socket_watcher_ref, void* arg, uint64_t event)
{
    LOG_FMT("watcher: %p arg: %p event %lx", socket_watcher_ref, arg, event);
    XrConnRef conn_ref = (XrConnRef)arg;
    XR_CONN_CHECK_TAG(conn_ref)
    XR_SOCKW_CHECK_TAG(socket_watcher_ref)
    XrReactorRef reactor_ref = socket_watcher_ref->runloop;
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
                // @TODO fix the next two lines
//                WSocket_change_watch(sw, conn_ref->write_completion_handler, arg, 0);
                LOG_FMT("write buffer complete %d", blen);
                WSocket_disarm_write(socket_watcher_ref);
                XrReactor_post(reactor_ref, &write_complete_post_func, arg);
                break;
            }
        } else {
            assert(nwrite < 0);
            if((saved_errno == EAGAIN) || (saved_errno == EWOULDBLOCK)) {
                conn_ref->write_rc = XRW_EAGAIN;
                return; // wait for the next write ready event, which will call this function to complete the buffer
            } else {
                conn_ref->write_rc = XRW_ERROR;
                // @TODO fix the next two lines
//                WSocket_change_watch(sw, conn_ref->write_completion_handler, arg, 0);
                WSocket_disarm_write(socket_watcher_ref);
                XrReactor_post(reactor_ref, &write_complete_post_func, arg);
                break;
            }
        }
    }
}
void XrConn_write_2(XrConnRef this, IOBufferRef buf, SocketEventHandler completion_handler)
{
    XR_CONN_CHECK_TAG(this)
    this->write_buffer_ref = buf;
//    this->write_completion_handler = completion_handler;
    WSocketRef sw = this->sock_watcher_ref;
    WSocket_arm_write(sw, &write_machine, (void*)this);
}