#include <c_http/xr/conn.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/epoll.h>


/*
 * **************************************************************************************************************************
 * XrConn_write
 * **************************************************************************************************************************
 */
static void write_cb_wrapper(XrWatcherRef wp, void* arg, uint64_t event)
{
    XrConnRef conn_ref = arg;
    XR_CONN_CHECK_TAG(conn_ref)
    XR_TRACE("conn_ref: %p arg: %p event: %lx", conn_ref, arg, event);
    conn_ref->write_cb(conn_ref, conn_ref->write_arg, 0);
}
static void write_event_handler(XrWatcherRef wp, void* arg, uint64_t event)
{
    XrSocketWatcherRef sw = (XrSocketWatcherRef)wp;
    XrConnRef conn_ref = arg;
    XR_CONN_CHECK_TAG(conn_ref)
    XrReactorRef reactor_ref = sw->runloop;
    XR_TRACE("conn_ref: %p", conn_ref);
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
    Xrsw_change_watch(sw, &write_event_handler, arg, 0);
    XrReactor_post(reactor_ref, wp, write_cb_wrapper, conn_ref);

}
void write_machine(XrWatcherRef wp, void* arg, uint64_t event);
void XrConn_write(XrConnRef this, IOBufferRef iobuf, XrConnWriteCallback cb, void* arg)
{
    XR_TRACE("conn_ref: %p iobuf: %p iobuf len: %d arg: %p", this, iobuf, IOBuffer_data_len(iobuf), arg);
    XR_CONN_CHECK_TAG(this)
    this->write_buffer_ref = iobuf;
    this->write_arg = arg;
    this->write_cb = cb;
    this->write_completion_handler = &write_cb_wrapper;
    XrSocketWatcherRef sw = this->sock_watcher_ref;
    Xrsw_change_watch(sw, &write_machine, this, EPOLLOUT|EPOLLERR);
}

void XrConn_prepare_write_2(XrConnRef this, IOBufferRef buf, XrSocketWatcherCallback completion_handler)
{
}
/**
 * This function is called by the Reactor on EPOLLOUT or EPOLLERR. It keeps getting called until
 * the entire buffer is written or an error occurs which is not an EAGAIN
 * \param wp    XrWatcherRef (but really an XrSocketWatcherRef)
 * \param arg   void* But typecast it to XrConnRef
 * \param event EPOLL event not used
 *
 * Returns result code XrWriteRC in XrConnRef->write_rc
 * and uses XrConnRef->write_???? properties as saved context between
 * invocations.
 *
 */
void write_machine(XrWatcherRef wp, void* arg, uint64_t event)
{
    XR_TRACE("watcher: %p arg: %p event %xl", wp, arg, event);
    XrSocketWatcherRef sw = (XrSocketWatcherRef)wp;
    XrConnRef conn_ref = (XrConnRef)arg;
    XR_CONN_CHECK_TAG(conn_ref)

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
                Xrsw_change_watch(sw, conn_ref->write_completion_handler, arg, 0);
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
                Xrsw_change_watch(sw, conn_ref->write_completion_handler, arg, 0);
                XrReactor_post(reactor_ref, wp, conn_ref->write_completion_handler, arg);
                break;
            }
        }
    }
}
void XrConn_write_2(XrConnRef this, IOBufferRef buf, XrSocketWatcherCallback completion_handler)
{
    XR_CONN_CHECK_TAG(this)
    this->write_buffer_ref = buf;
    this->write_completion_handler = completion_handler;
    XrSocketWatcherRef sw = this->sock_watcher_ref;
    Xrsw_change_watch(sw, &write_machine, (void*)this, EPOLLERR | EPOLLOUT);
}