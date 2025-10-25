#include "runloop_internal.h"
#include <rbl/logger.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>
#include "epoll_helper.h"

/**
 * Called whenever an fd associated with an WSocket receives an fd event.
 * Should dispatch the read_evhandler and/or write_evhandler depending on whether those
 * events (read events and write events) are armed.
 * @param ctx       void*
 * @param fd        int
 * @param event     uint64_t
 */
static void handler(RunloopWatcherBaseRef watcher, uint64_t event)
{
    RunloopStreamRef rl_stream = (RunloopStreamRef)watcher;
    RunloopRef rl = watcher->runloop;
    /*
     * These should be posted not called
     */
    if(rl_stream->event_mask & EPOLLIN) {
        RBL_LOG_FMT("handler runloop_stream POLLIN fd: %d \n", rl_stream->fd);
    }
    if(rl_stream->event_mask & EPOLLOUT) {
        RBL_LOG_FMT("handler runloop_stream POLLOUT fd: %d \n", rl_stream->fd);
    }
    if((rl_stream->event_mask & EPOLLIN) && (rl_stream->read_postable_cb)) {
        rl_stream->read_postable_cb(rl, rl_stream->read_postable_arg);
    }
    if((rl_stream->event_mask & EPOLLOUT) && (rl_stream->write_postable_cb)) {
        rl_stream->write_postable_cb(rl, rl_stream->write_postable_arg);
    }
}

static void anonymous_free(RunloopWatcherBaseRef p)
{
    RunloopStreamRef twp = (RunloopStreamRef)p;
    runloop_stream_free(twp);
}
void runloop_stream_init(RunloopStreamRef this, RunloopRef runloop, int fd)
{
    STREAM_SET_TAG(this);
    STREAM_SET_END_TAG(this);
    this->fd = fd;
    this->runloop = runloop;
    this->free = &anonymous_free;
    this->handler = &handler;
    this->event_mask = 0;
    this->read_postable_arg = NULL;
    this->read_postable_cb = NULL;
    this->write_postable_arg = NULL;
    this->write_postable_cb = NULL;
}
RunloopStreamRef runloop_stream_new(RunloopRef runloop, int fd)
{
    RunloopStreamRef this = rl_event_allocate(runloop, sizeof(RunloopStream));
    runloop_stream_init(this, runloop, fd);
    return this;
}
void runloop_stream_free(RunloopStreamRef athis)
{
    STREAM_SET_TAG(athis);
    STREAM_SET_END_TAG(athis);
    close(athis->fd);
    rl_event_free(athis->runloop, athis);
}
void runloop_stream_register(RunloopStreamRef athis)
{
    STREAM_SET_TAG(athis);
    STREAM_SET_END_TAG(athis);
    uint32_t interest = 0;
    eph_add(athis->runloop->epoll_fd, athis->fd, 0L, athis);
//    int res = runloop_register(athis->runloop, athis->fd, 0L, (RunloopWatcherBaseRef) (athis));
//    assert(res ==0);
}
//void WIoFd_change_watch(RunloopStreamRef this, SocketEventHandler cb, void* arg, uint64_t watch_what)
//{
//    STREAM_CHECK_TAG(this)
//    uint32_t interest = watch_what;
//    if( cb != NULL) {
//        this->cb = cb;
//    }
//    if (arg != NULL) {
//        this->cb_ctx = arg;
//    }
//    int res = runloop_reregister(this->runloop, this->fd, interest, (RunloopWatcherBaseRef)this);
//    assert(res == 0);
//}
void runloop_stream_deregister(RunloopStreamRef athis)
{
    STREAM_SET_TAG(athis);
    STREAM_SET_END_TAG(athis);
    eph_del(athis->runloop->epoll_fd, athis->fd, 0, athis);
//    int res = runloop_deregister(athis->runloop, athis->fd);
//    assert(res == 0);
}
void runloop_stream_arm_both(RunloopStreamRef athis,
                             PostableFunction read_postable_cb, void* read_arg,
                             PostableFunction write_postable_cb, void* write_arg)
{
    uint64_t interest = EPOLLET | EPOLLOUT | EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLRDHUP | athis->event_mask;
    athis->event_mask = interest;
    STREAM_SET_TAG(athis);
    STREAM_SET_END_TAG(athis);
    if(read_postable_cb != NULL) {
        athis->read_postable_cb = read_postable_cb;
    }
    if (read_arg != NULL) {
        athis->read_postable_arg = read_arg;
    }
    if(write_postable_cb != NULL) {
        athis->write_postable_cb = write_postable_cb;
    }
    if (write_arg != NULL) {
        athis->write_postable_arg = write_arg;
    }
    eph_mod(athis->runloop->epoll_fd, athis->fd, interest, athis);
}

void runloop_stream_arm_read(RunloopStreamRef athis, PostableFunction postable_cb, void* arg)
{
    uint64_t interest = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLRDHUP | athis->event_mask;
    athis->event_mask = interest;
    STREAM_SET_TAG(athis);
    STREAM_SET_END_TAG(athis);
    if(postable_cb != NULL) {
        athis->read_postable_cb = postable_cb;
    }
    if (arg != NULL) {
        athis->read_postable_arg = arg;
    }
    eph_mod(athis->runloop->epoll_fd, athis->fd, interest, athis);
}
void runloop_stream_arm_write(RunloopStreamRef athis, PostableFunction postable_cb, void* arg)
{
    uint64_t interest = EPOLLOUT | EPOLLERR | EPOLLHUP | EPOLLRDHUP | athis->event_mask;
    athis->event_mask = interest;
    STREAM_SET_TAG(athis);
    STREAM_SET_END_TAG(athis);
    if(postable_cb != NULL) {
        athis->write_postable_cb = postable_cb;
    }
    if (arg != NULL) {
        athis->write_postable_arg = arg;
    }
    eph_mod(athis->runloop->epoll_fd, athis->fd, interest, athis);
}
void runloop_stream_disarm_read(RunloopStreamRef athis)
{
    athis->event_mask &= ~EPOLLIN;
    STREAM_SET_TAG(athis);
    STREAM_SET_END_TAG(athis);
    athis->read_postable_cb = NULL;
    athis->read_postable_arg = NULL;
    eph_mod(athis->runloop->epoll_fd, athis->fd, athis->event_mask, athis);
}
void runloop_stream_disarm_write(RunloopStreamRef athis)
{
    athis->event_mask = ~EPOLLOUT & athis->event_mask;
    STREAM_SET_TAG(athis);
    STREAM_SET_END_TAG(athis);
    athis->write_postable_cb = NULL;
    athis->write_postable_arg = NULL;
    eph_mod(athis->runloop->epoll_fd, athis->fd, athis->event_mask, athis);
}
RunloopRef runloop_stream_get_runloop(RunloopStreamRef athis)
{
    STREAM_SET_TAG(athis);
    STREAM_SET_END_TAG(athis);
    return athis->runloop;
}
int runloop_stream_get_fd(RunloopStreamRef athis)
{
    STREAM_SET_TAG(athis);
    STREAM_SET_END_TAG(athis);
    return athis->fd;
}

void runloop_stream_verify(RunloopStreamRef athis)
{
    STREAM_SET_TAG(athis);
    STREAM_SET_END_TAG(athis);
}
void runloop_stream_checktag(RunloopStreamRef athis)
{
    STREAM_SET_TAG(athis);
    STREAM_SET_END_TAG(athis);
}



