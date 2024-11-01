#include <http_in_c//runloop/runloop.h>
#include <http_in_c/runloop/rl_internal.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>

/**
 * Called whenever an fd associated with an WSocket receives an fd event.
 * Should dispatch the read_evhandler and/or write_evhandler depending on whether those
 * events (read events and write events) are armed.
 * @param ctx       void*
 * @param fd        int
 * @param event     uint64_t
 */
static void handler(RtorWatcherRef watcher, uint64_t event)
{
    RtorStreamRef sw = (RtorStreamRef)watcher;
    if(sw->both_handler) {
        sw->both_handler(sw, event);
    }
    if((sw->event_mask & EPOLLIN) && (sw->read_evhandler)) {
        sw->read_evhandler(sw, event);
    }
    if((sw->event_mask & EPOLLOUT) && (sw->write_evhandler)) {
        sw->write_evhandler(sw, event);
    }
}

static void anonymous_free(RtorWatcherRef p)
{
    RtorStreamRef twp = (RtorStreamRef)p;
    rtor_stream_free(twp);
}
void rtor_stream_init(RtorStreamRef this, ReactorRef runloop, int fd)
{
    RBL_SET_TAG(TYPE, this)
    SOCKW_SET_TAG(this);
    this->fd = fd;
    this->runloop = runloop;
    this->free = &anonymous_free;
    this->handler = &handler;
    this->event_mask = 0;
    this->read_arg = NULL;
    this->read_evhandler = NULL;
    this->write_arg = NULL;
    this->write_evhandler = NULL;
}
RtorStreamRef rtor_stream_new(ReactorRef runloop, int fd)
{
    RtorStreamRef this = malloc(sizeof(RtorStream));
    rtor_stream_init(this, runloop, fd);
    return this;
}
void rtor_stream_free(RtorStreamRef athis)
{
    SOCKW_CHECK_TAG(athis)
    close(athis->fd);
    free((void*)athis);
}
void rtor_stream_register(RtorStreamRef athis)
{
    SOCKW_CHECK_TAG(athis)
    uint32_t interest = 0;
    int res = rtor_reactor_register(athis->runloop, athis->fd, 0L, (RtorWatcherRef) (athis));
    assert(res ==0);
}
//void WIoFd_change_watch(RtorStreamRef this, SocketEventHandler cb, void* arg, uint64_t watch_what)
//{
//    SOCKW_CHECK_TAG(this)
//    uint32_t interest = watch_what;
//    if( cb != NULL) {
//        this->cb = cb;
//    }
//    if (arg != NULL) {
//        this->cb_ctx = arg;
//    }
//    int res = rtor_reactor_reregister(this->runloop, this->fd, interest, (RtorWatcherRef)this);
//    assert(res == 0);
//}
void rtor_stream_deregister(RtorStreamRef athis)
{
    SOCKW_CHECK_TAG(athis)
    int res = rtor_reactor_deregister(athis->runloop, athis->fd);
    assert(res == 0);
}
void rtor_stream_arm_both(RtorStreamRef athis, SocketEventHandler event_handler, void* arg)
{
    uint64_t interest = EPOLLET | EPOLLOUT | EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLRDHUP | athis->event_mask;
    athis->event_mask = interest;
    SOCKW_CHECK_TAG(athis)
    if(event_handler != NULL) {
        athis->both_handler = event_handler;
    }
    if (arg != NULL) {
        athis->both_arg = arg;
    }
    int res = rtor_reactor_reregister(athis->runloop, athis->fd, interest, (RtorWatcherRef) athis);
    assert(res == 0);
}

void rtor_stream_arm_read(RtorStreamRef athis, SocketEventHandler event_handler, void* arg)
{
    uint64_t interest = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLRDHUP | athis->event_mask;
    athis->event_mask = interest;
    SOCKW_CHECK_TAG(athis)
    if(event_handler != NULL) {
        athis->read_evhandler = event_handler;
    }
    if (arg != NULL) {
        athis->read_arg = arg;
    }
    int res = rtor_reactor_reregister(athis->runloop, athis->fd, interest, (RtorWatcherRef) athis);
    assert(res == 0);
}
void rtor_stream_arm_write(RtorStreamRef athis, SocketEventHandler event_handler, void* arg)
{
    uint64_t interest = EPOLLOUT | EPOLLERR | EPOLLHUP | EPOLLRDHUP | athis->event_mask;
    athis->event_mask = interest;
    SOCKW_CHECK_TAG(athis)
    if(event_handler != NULL) {
        athis->write_evhandler = event_handler;
    }
    if (arg != NULL) {
        athis->write_arg = arg;
    }
    int res = rtor_reactor_reregister(athis->runloop, athis->fd, interest, (RtorWatcherRef) athis);
    assert(res == 0);
}
void rtor_stream_disarm_read(RtorStreamRef athis)
{
    athis->event_mask &= ~EPOLLIN;
    SOCKW_CHECK_TAG(athis)
    athis->read_evhandler = NULL;
    athis->read_arg = NULL;
    int res = rtor_reactor_reregister(athis->runloop, athis->fd, athis->event_mask, (RtorWatcherRef) athis);
    assert(res == 0);
}
void rtor_stream_disarm_write(RtorStreamRef athis)
{
    athis->event_mask = ~EPOLLOUT & athis->event_mask;
    SOCKW_CHECK_TAG(athis)
    int res = rtor_reactor_reregister(athis->runloop, athis->fd, athis->event_mask, (RtorWatcherRef) athis);
    assert(res == 0);
}
ReactorRef rtor_stream_get_reactor(RtorStreamRef athis)
{
    return athis->runloop;
}
int rtor_stream_get_fd(RtorStreamRef this)
{
    return this->fd;
}

void rtor_stream_verify(RtorStreamRef r)
{
    SOCKW_CHECK_TAG(r)

}


