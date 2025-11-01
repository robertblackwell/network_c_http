#include <kqueue_runloop/runloop_internal.h>
#include <rbl/logger.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

/**
 * Called whenever an fd associated with an WSocket receives an fd event.
 * Should dispatch the read_evhandler and/or write_evhandler depending on whether those
 * events (read events and write events) are armed.
 * @param ctx       void*
 * @param fd        int
 * @param event     uint64_t
 */
static void handler(RunloopEventRef watcher, uint16_t filter, uint16_t flags)
{
    RunloopStreamRef rl_stream = (RunloopStreamRef)watcher;
    RunloopRef rl = watcher->runloop;
    /*
     * Act on the kqueue filter
     */
    int16_t int_filter = (int16_t)(filter);
    switch(int_filter) {
        case EVFILT_READ:
            printf("read event %ld\n", (long)(int16_t)filter);
            if(watcher->stream.read_postable_cb != NULL) {
                watcher->stream.read_postable_cb(rl, watcher->stream.read_postable_arg);
            } else {
                printf("read event no postable\n");
            }
            break;
        case EVFILT_WRITE:
            printf("write event %ld\n", (long)(int64_t)filter);
            if(watcher->stream.write_postable_cb != NULL) {
                watcher->stream.write_postable_cb(rl, watcher->stream.write_postable_arg);
            } else {
                printf("write event no postable\n");
            }
            break;
        default:
            printf("unknown event %ld\n", (long)(int64_t)filter);
            assert(false);
            break;
    }
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

static void anonymous_free(RunloopStreamRef p)
{
    runloop_stream_free(p);
}
void runloop_stream_init(RunloopStreamRef this, RunloopRef runloop, int fd)
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
void runloop_stream_deinit(RunloopStreamRef this)
{
    SOCKW_SET_TAG(this);
    SOCKW_SET_END_TAG(this);
    this->stream.fd = 0;
    this->runloop = NULL;
    this->free = &anonymous_free;
    this->handler = NULL;
    this->stream.event_mask = 0;
    this->stream.read_postable_arg = NULL;
    this->stream.read_postable_cb = NULL;
    this->stream.write_postable_arg = NULL;
    this->stream.write_postable_cb = NULL;
}

RunloopStreamRef runloop_stream_new(RunloopRef runloop, int fd)
{
    RunloopStreamRef this = event_table_get_entry(runloop->event_table);
    runloop_stream_init(this, runloop, fd);
    return this;
}
void runloop_stream_free(RunloopStreamRef athis)
{
    SOCKW_SET_TAG(athis);
    SOCKW_SET_END_TAG(athis);
    runloop_stream_deregister(athis);
    close(athis->stream.fd);
    event_table_release_entry(athis->runloop->event_table, athis);
}
void runloop_stream_register(RunloopStreamRef athis)
{
    SOCKW_SET_TAG(athis);
    SOCKW_SET_END_TAG(athis);
    int res = kqh_readerwriter_register(athis);
    assert(res == 0);
    // res = kqh_readerwriter_pause(athis);
    assert(res == 0);
}
void runloop_stream_deregister(RunloopStreamRef athis)
{
    SOCKW_SET_TAG(athis);
    SOCKW_SET_END_TAG(athis);
    int res = kqh_readerwriter_cancel(athis);
    assert(res == 0);
}
void runloop_stream_arm_both(RunloopStreamRef athis,
                             PostableFunction read_postable_cb, void* read_arg,
                             PostableFunction write_postable_cb, void* write_arg)
{
    // athis->stream.event_mask = interest;
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
    int res = kqh_readerwriter_register(athis);
    assert(res == 0);
}

void runloop_stream_arm_read(RunloopStreamRef athis, PostableFunction postable_cb, void* arg)
{
    SOCKW_SET_TAG(athis);
    SOCKW_SET_END_TAG(athis);
    if(postable_cb != NULL) {
        athis->stream.read_postable_cb = postable_cb;
    }
    if (arg != NULL) {
        athis->stream.read_postable_arg = arg;
    }
    int res = kqh_reader_register(athis);
    assert(res == 0);
}
void runloop_stream_arm_write(RunloopStreamRef athis, PostableFunction postable_cb, void* arg)
{
    SOCKW_SET_TAG(athis);
    SOCKW_SET_END_TAG(athis);
    if(postable_cb != NULL) {
        athis->stream.write_postable_cb = postable_cb;
    }
    if (arg != NULL) {
        athis->stream.write_postable_arg = arg;
    }
    int res = kqh_writer_register(athis);
    assert(res == 0);
}
void runloop_stream_disarm_read(RunloopStreamRef athis)
{
    SOCKW_SET_TAG(athis);
    SOCKW_SET_END_TAG(athis);
    athis->stream.read_postable_cb = NULL;
    athis->stream.read_postable_arg = NULL;
    int res = kqh_reader_pause(athis);
    assert(res == 0);
}
void runloop_stream_disarm_write(RunloopStreamRef athis)
{
    athis->stream.event_mask = 0;//~EPOLLOUT & athis->event_mask;
    SOCKW_SET_TAG(athis);
    SOCKW_SET_END_TAG(athis);
    athis->stream.write_postable_cb = NULL;
    athis->stream.write_postable_arg = NULL;
    int res = kqh_writer_pause(athis);
    assert(res == 0);
}
RunloopRef runloop_stream_get_runloop(RunloopStreamRef athis)
{
    SOCKW_SET_TAG(athis);
    SOCKW_SET_END_TAG(athis);
    return athis->runloop;
}
int runloop_stream_get_fd(RunloopStreamRef athis)
{
    SOCKW_SET_TAG(athis);
    SOCKW_SET_END_TAG(athis);
    return athis->stream.fd;
}

void runloop_stream_verify(RunloopStreamRef athis)
{
    SOCKW_SET_TAG(athis);
    SOCKW_SET_END_TAG(athis);
}
void runloop_stream_checktag(RunloopStreamRef athis)
{
    SOCKW_SET_TAG(athis);
    SOCKW_SET_END_TAG(athis);
}



