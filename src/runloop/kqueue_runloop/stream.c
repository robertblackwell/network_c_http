#include <kqueue_runloop/runloop.h>
#include <kqueue_runloop/rl_internal.h>
#include <rbl/logger.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * Called whenever an fd associated with an WSocket receives an fd event.
 * Should dispatch the read_evhandler and/or write_evhandler depending on whether those
 * events (read events and write events) are armed.
 * @param ctx       void*
 * @param fd        int
 * @param event     uint64_t
 */
static void handler(RunloopEventRef watcher, uint64_t event)
{
    RunloopEventRef rl_stream = (RunloopEventRef)watcher;
    RunloopRef rl = watcher->runloop;
    /*
     * These should be posted not called
     */
    if(rl_stream->stream.event_mask /*& EPOLLIN*/) {
        RBL_LOG_FMT("handler runloop_stream POLLIN fd: %d \n", rl_stream->stream.fd);
    }
    if(rl_stream->stream.event_mask /*& EPOLLOUT*/) {
        RBL_LOG_FMT("handler runloop_stream POLLOUT fd: %d \n", rl_stream->stream.fd);
    }
    if((rl_stream->stream.event_mask /*& EPOLLIN*/) && (rl_stream->stream.read_postable_cb)) {
        rl_stream->stream.read_postable_cb(rl, rl_stream->stream.read_postable_arg);
    }
    if((rl_stream->stream.event_mask /*& EPOLLOUT*/) && (rl_stream->stream.write_postable_cb)) {
        rl_stream->stream.write_postable_cb(rl, rl_stream->stream.write_postable_arg);
    }
}

static void anonymous_free(RunloopEventRef p)
{
    runloop_stream_free(p);
}
void runloop_stream_init(RunloopEventRef this, RunloopRef runloop, int fd)
{
    SOCKW_SET_TAG(this);
    SOCKW_SET_END_TAG(this);
    this->stream.fd = fd;
    this->runloop = runloop;
    this->free = &anonymous_free;
    this->handler = &handler;
    this->stream.event_mask = 0;
    this->stream.read_postable_arg = NULL;
    this->stream.read_postable_cb = NULL;
    this->stream.write_postable_arg = NULL;
    this->stream.write_postable_cb = NULL;
}
RunloopEventRef runloop_stream_new(RunloopRef runloop, int fd)
{
    RunloopEventRef this = malloc(sizeof(RunloopStream));
    runloop_stream_init(this, runloop, fd);
    return this;
}
void runloop_stream_free(RunloopEventRef athis)
{
    SOCKW_SET_TAG(athis);
    SOCKW_SET_END_TAG(athis);
    close(athis->stream.fd);
    free((void*)athis);
}
void runloop_stream_register(RunloopEventRef athis)
{
    SOCKW_SET_TAG(athis);
    SOCKW_SET_END_TAG(athis);
    uint32_t interest = 0;
    int res = runloop_register(athis->runloop, athis->stream.fd, 0L, (RunloopWatcherBaseRef) (athis));
    assert(res ==0);
}
//void WIoFd_change_watch(RunloopEventRef this, SocketEventHandler cb, void* arg, uint64_t watch_what)
//{
//    SOCKW_CHECK_TAG(this)
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
void runloop_stream_deregister(RunloopEventRef athis)
{
    SOCKW_SET_TAG(athis);
    SOCKW_SET_END_TAG(athis);
    int res = runloop_deregister(athis->runloop, athis->stream.fd);
    assert(res == 0);
}
void runloop_stream_arm_both(RunloopEventRef athis,
                             PostableFunction read_postable_cb, void* read_arg,
                             PostableFunction write_postable_cb, void* write_arg)
{
    uint64_t interest = 0;//EPOLLET | EPOLLOUT | EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLRDHUP | athis->event_mask;
    athis->stream.event_mask = interest;
    SOCKW_SET_TAG(athis);
    SOCKW_SET_END_TAG(athis);
    if(read_postable_cb != NULL) {
        athis->stream.read_postable_cb = read_postable_cb;
    }
    if (read_arg != NULL) {
        athis->stream.read_postable_arg = read_arg;
    }
    if(write_postable_cb != NULL) {
        athis->stream.write_postable_cb = write_postable_cb;
    }
    if (write_arg != NULL) {
        athis->stream.write_postable_arg = write_arg;
    }
    int res = runloop_reregister(athis->runloop, athis->stream.fd, interest, (RunloopWatcherBaseRef) athis);
    assert(res == 0);
}

void runloop_stream_arm_read(RunloopEventRef athis, PostableFunction postable_cb, void* arg)
{
    uint64_t interest = 0;//EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLRDHUP | athis->event_mask;
    athis->stream.event_mask = interest;
    SOCKW_SET_TAG(athis);
    SOCKW_SET_END_TAG(athis);
    if(postable_cb != NULL) {
        athis->stream.read_postable_cb = postable_cb;
    }
    if (arg != NULL) {
        athis->stream.read_postable_arg = arg;
    }
    int res = runloop_reregister(athis->runloop, athis->stream.fd, interest, (RunloopWatcherBaseRef) athis);
    assert(res == 0);
}
void runloop_stream_arm_write(RunloopEventRef athis, PostableFunction postable_cb, void* arg)
{
    uint64_t interest = 0;//EPOLLOUT | EPOLLERR | EPOLLHUP | EPOLLRDHUP | athis->event_mask;
    athis->stream.event_mask = interest;
    SOCKW_SET_TAG(athis);
    SOCKW_SET_END_TAG(athis);
    if(postable_cb != NULL) {
        athis->stream.write_postable_cb = postable_cb;
    }
    if (arg != NULL) {
        athis->stream.write_postable_arg = arg;
    }
    int res = runloop_reregister(athis->runloop, athis->stream.fd, interest, (RunloopWatcherBaseRef) athis);
    assert(res == 0);
}
void runloop_stream_disarm_read(RunloopEventRef athis)
{
    athis->stream.event_mask &= 0;//~EPOLLIN;
    SOCKW_SET_TAG(athis);
    SOCKW_SET_END_TAG(athis);
    athis->stream.read_postable_cb = NULL;
    athis->stream.read_postable_arg = NULL;
    int res = runloop_reregister(athis->runloop, athis->stream.fd, athis->stream.event_mask, (RunloopWatcherBaseRef) athis);
    assert(res == 0);
}
void runloop_stream_disarm_write(RunloopEventRef athis)
{
    athis->stream.event_mask = 0;//~EPOLLOUT & athis->event_mask;
    SOCKW_SET_TAG(athis);
    SOCKW_SET_END_TAG(athis);
    athis->stream.write_postable_cb = NULL;
    athis->stream.write_postable_arg = NULL;
    int res = runloop_reregister(athis->runloop, athis->stream.fd, athis->stream.event_mask, (RunloopWatcherBaseRef) athis);
    assert(res == 0);
}
RunloopRef runloop_stream_get_runloop(RunloopEventRef athis)
{
    SOCKW_SET_TAG(athis);
    SOCKW_SET_END_TAG(athis);
    return athis->runloop;
}
int runloop_stream_get_fd(RunloopEventRef athis)
{
    SOCKW_SET_TAG(athis);
    SOCKW_SET_END_TAG(athis);
    return athis->stream.fd;
}

void runloop_stream_verify(RunloopEventRef athis)
{
    SOCKW_SET_TAG(athis);
    SOCKW_SET_END_TAG(athis);
}
void runloop_stream_checktag(RunloopEventRef athis)
{
    SOCKW_SET_TAG(athis);
    SOCKW_SET_END_TAG(athis);
}



