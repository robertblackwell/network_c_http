#include "runloop_internal.h"
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
static void handler(RunloopEventBaseRef watcher, uint16_t filter, uint16_t flags, void* data)
{
    RunloopStreamRef stream = (RunloopStreamRef)watcher;
    RunloopRef rl = watcher->runloop;
    /*
     * Act on the kqueue filter
     */
    int16_t int_filter = (int16_t)(filter);
    switch(int_filter) {
        case EVFILT_READ:
            printf("read event fd:%d filter: %ld   flags: %d\n", stream->fd, (long)(int16_t)filter, (int)flags);
            if(stream->read_postable_cb != NULL) {
                stream->read_postable_cb(rl, stream->read_postable_arg);
            } else {
                printf("read event no postable\n");
            }
            break;
        case EVFILT_WRITE:
            printf("write event %ld\n", (long)(int64_t)filter);
            if(stream->write_postable_cb != NULL) {
                stream->write_postable_cb(rl, stream->write_postable_arg);
            } else {
                printf("write event no postable\n");
            }
            break;
        default:
            printf("unknown event %ld\n", (long)(int64_t)filter);
            assert(false);
            break;
    }
    if(stream->event_mask /*& EPOLLIN*/) {
        RBL_LOG_FMT("handler runloop_stream POLLIN fd: %d \n", rl_stream->fd);
    }
    if(stream->event_mask /*& EPOLLOUT*/) {
        RBL_LOG_FMT("handler runloop_stream POLLOUT fd: %d \n", rl_stream->fd);
    }
    if((stream->event_mask /*& EPOLLIN*/) && (stream->read_postable_cb)) {
        stream->read_postable_cb(rl, stream->read_postable_arg);
    }
    if((stream->event_mask /*& EPOLLOUT*/) && (stream->write_postable_cb)) {
        stream->write_postable_cb(rl, stream->write_postable_arg);
    }
}

static void anonymous_free(RunloopStreamRef p)
{
    runloop_stream_free(p);
}
void runloop_stream_init(RunloopStreamRef stream, RunloopRef runloop, int fd)
{
    STREAM_SET_TAG(stream);
    STREAM_SET_END_TAG(stream);
    stream->fd = fd;
    stream->runloop = runloop;
    stream->handler = &handler;
    stream->event_mask = 0;
    stream->read_postable_arg = NULL;
    stream->read_postable_cb = NULL;
    stream->write_postable_arg = NULL;
    stream->write_postable_cb = NULL;
}
void runloop_stream_deinit(RunloopStreamRef stream)
{
    STREAM_SET_TAG(stream);
    STREAM_SET_END_TAG(stream);
    stream->fd = 0;
    stream->runloop = NULL;
    stream->handler = NULL;
    stream->event_mask = 0;
    stream->read_postable_arg = NULL;
    stream->read_postable_cb = NULL;
    stream->write_postable_arg = NULL;
    stream->write_postable_cb = NULL;
}

RunloopStreamRef runloop_stream_new(RunloopRef runloop, int fd)
{
    RunloopStreamRef stream = runloop_event_allocate(runloop, sizeof(RunloopStream));
    runloop_stream_init(stream, runloop, fd);
    return stream;
}
void runloop_stream_free(RunloopStreamRef stream)
{
    STREAM_SET_TAG(stream);
    STREAM_SET_END_TAG(stream);
    runloop_stream_deregister(stream);
    close(stream->fd);
    runloop_event_free(stream->runloop, stream);
}
void runloop_stream_register(RunloopStreamRef stream)
{
    STREAM_SET_TAG(stream);
    STREAM_SET_END_TAG(stream);
    int res = kqh_readerwriter_register(stream);
    assert(res == 0);
    // res = kqh_readerwriter_pause(stream);
    assert(res == 0);
}
void runloop_stream_deregister(RunloopStreamRef stream)
{
    STREAM_SET_TAG(stream);
    STREAM_SET_END_TAG(stream);
    int res = kqh_readerwriter_cancel(stream);
    assert(res == 0);
}
void runloop_stream_arm_both(RunloopStreamRef stream,
                             PostableFunction read_postable_cb, void* read_arg,
                             PostableFunction write_postable_cb, void* write_arg)
{
    // stream->event_mask = interest;
    STREAM_SET_TAG(stream);
    STREAM_SET_END_TAG(stream);
    if(read_postable_cb != NULL) {
        stream->read_postable_cb = read_postable_cb;
    }
    if (read_arg != NULL) {
        stream->read_postable_arg = read_arg;
    }
    if(write_postable_cb != NULL) {
        stream->write_postable_cb = write_postable_cb;
    }
    if (write_arg != NULL) {
        stream->write_postable_arg = write_arg;
    }
    int res = kqh_readerwriter_register(stream);
    assert(res == 0);
}

void runloop_stream_arm_read(RunloopStreamRef stream, PostableFunction postable_cb, void* arg)
{
    STREAM_SET_TAG(stream);
    STREAM_SET_END_TAG(stream);
    if(postable_cb != NULL) {
        stream->read_postable_cb = postable_cb;
    }
    if (arg != NULL) {
        stream->read_postable_arg = arg;
    }
    int res = kqh_reader_register(stream);
    assert(res == 0);
}
void runloop_stream_arm_write(RunloopStreamRef stream, PostableFunction postable_cb, void* arg)
{
    STREAM_SET_TAG(stream);
    STREAM_SET_END_TAG(stream);
    if(postable_cb != NULL) {
        stream->write_postable_cb = postable_cb;
    }
    if (arg != NULL) {
        stream->write_postable_arg = arg;
    }
    int res = kqh_writer_register(stream);
    assert(res == 0);
}
void runloop_stream_disarm_read(RunloopStreamRef stream)
{
    STREAM_SET_TAG(stream);
    STREAM_SET_END_TAG(stream);
    stream->read_postable_cb = NULL;
    stream->read_postable_arg = NULL;
    int res = kqh_reader_pause(stream);
    assert(res == 0);
}
void runloop_stream_disarm_write(RunloopStreamRef stream)
{
    stream->event_mask = 0;//~EPOLLOUT & stream->event_mask;
    STREAM_SET_TAG(stream);
    STREAM_SET_END_TAG(stream);
    stream->write_postable_cb = NULL;
    stream->write_postable_arg = NULL;
    int res = kqh_writer_pause(stream);
    assert(res == 0);
}
RunloopRef runloop_stream_get_runloop(RunloopStreamRef stream)
{
    STREAM_SET_TAG(stream);
    STREAM_SET_END_TAG(stream);
    return stream->runloop;
}
int runloop_stream_get_fd(RunloopStreamRef stream)
{
    STREAM_SET_TAG(stream);
    STREAM_SET_END_TAG(stream);
    return stream->fd;
}

void runloop_stream_verify(RunloopStreamRef stream)
{
    STREAM_SET_TAG(stream);
    STREAM_SET_END_TAG(stream);
}
void runloop_stream_checktag(RunloopStreamRef stream)
{
    STREAM_SET_TAG(stream);
    STREAM_SET_END_TAG(stream);
}



