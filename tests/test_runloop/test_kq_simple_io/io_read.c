#include "io_read.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <sys/event.h>
#include <rbl/logger.h>
#include <src/common/utils.h>
#include <kqueue_runloop/runloop_internal.h>

void async_socket_set_nonblocking(int socket);
static void try_read(ReadCtx* ctx);
static void read_clean_termination(ReadCtx* ctx);
static void read_error_termination(ReadCtx* ctx, int err);
static void postable_reader(RunloopRef rl, void* arg);

/**
 * the reader does the following
 *
 *  initialization is set a watcher to be called when the fd is readable
 *
 *      when readable read a message into a buffer
 *      print the message or if io error print "badread"
 *
 */
void ReadCtx_init(ReadCtx* this, int my_index, int fd, int max)
{
    RBL_SET_TAG(ReadCtx_TAG, this)
    RBL_SET_END_TAG(ReadCtx_TAG, this)
    this->id = "READ";
    this->read_count = 0;
    this->max_read_count = max;
    this->readfd = fd;
    this->reader_index = my_index;
    this->reader = NULL;
}

void ReaderTable_init(ReaderTable* this)
{
    RBL_SET_TAG(ReaderTable_TAG, this)
    RBL_SET_END_TAG(ReaderTable_TAG, this)
    this->count = 0;
}
ReaderTable* ReaderTable_new()
{
    ReaderTable* tmp = malloc(sizeof(ReaderTable));
    ReaderTable_init(tmp);
    return tmp;
}
void ReaderTable_free(ReaderTable* this)
{
    RBL_CHECK_TAG(ReaderTable_TAG, this)
    RBL_CHECK_END_TAG(ReaderTable_TAG, this)
    free(this);
}
void ReaderTable_add_fd(ReaderTable* this, int fd, int max)
{
    RBL_CHECK_TAG(ReaderTable_TAG, this)
    RBL_CHECK_END_TAG(ReaderTable_TAG, this)
    async_socket_set_nonblocking(fd);
    ReadCtxRef ctx = &(this->ctx_table[this->count]);
    ReadCtx_init(ctx, this->count, fd, max);
    this->count++;

}
void read_callback2(RunloopRef rl, void* arg)
{
    RunloopEventRef ev = arg;
    int fd = ev->stream.fd;
    printf("read_callback_2\n");
    char buffer[1000];
    int n = read(fd, buffer, sizeof(buffer));
    buffer[n] = '\0';
    printf("read succeeded n: %d s: %s\n", n, buffer);
}
void read_callback(RunloopRef rl, void* read_ctx_ref_arg)
{
    ReadCtx* ctx = (ReadCtx*)read_ctx_ref_arg;
    RunloopStreamRef stream = ctx->reader;
    RunloopRef runloop_ref = runloop_stream_get_runloop(stream);
    RBL_LOG_FMT("read_callback fd %d read_state: %d\n", ctx->readfd, ctx->read_state);
    switch(ctx->read_state){
        case RD_STATE_INITIAL:
        case RD_STATE_READY:
        case RD_STATE_EAGAIN:
            ctx->read_state = RD_STATE_READY;
            try_read(ctx);
            break;
        case RD_STATE_ERROR:
            break;
        case RD_STATE_STOPPED:
            break;
        default:
            assert(false);
    }
}
static void try_read(ReadCtx* ctx)
{
    RunloopStreamRef stream = ctx->reader;
    RunloopRef runloop_ref = runloop_stream_get_runloop(stream);
    assert(ctx->readfd == runloop_stream_get_fd(stream));
    RBL_LOG_FMT("try_read: read_state %d fd: %d \n", ctx->read_state, ctx->readfd);

    char buf[100000];
    int x = sizeof(buf);
    memset(buf, 0, sizeof(buf));
    int fd = runloop_stream_get_fd(stream);
    int nread = read(fd, buf, 100000);
    int errno_saved = errno;
    char* s;
    if(nread > 0) {
        buf[nread] = (char)0;
        s = &(buf[0]);
        ctx->read_state = RD_STATE_READY;
        ctx->read_count++;
        RBL_LOG_FMT("try_read nread: %d read_count: %d read_state: %d\n", nread, ctx->read_count, ctx->read_state);
        if(ctx->read_count > ctx->max_read_count) {
            read_clean_termination(ctx);
        } 
        runloop_post(runloop_ref, postable_reader, ctx);
    } else {
        if(errno_saved == EAGAIN) {
            RBL_LOG_FMT("try_read ERROR EAGAIN %s\n", "");
            ctx->read_state = RD_STATE_EAGAIN;
            runloop_stream_arm_read(ctx->reader, read_callback, ctx);
        } else {
            RBL_LOG_FMT("try_read ERROR errno: %d description %s\n", errno_saved, strerror(errno_saved));
            read_error_termination(ctx, errno_saved);
        }
    }
}
static void postable_reader(RunloopRef rl, void* arg)
{
    ReadCtx* ctx = arg;
    try_read(ctx);
}
static void read_clean_termination(ReadCtx* ctx)
{
    RBL_LOG_FMT("read_clean_termination fd %d read_state: %d\n", ctx->readfd, ctx->read_state);
    ctx->read_state = RD_STATE_STOPPED;
    ctx->return_code = 0;
    runloop_stream_deregister(ctx->reader);
}
static void read_error_termination(ReadCtx* ctx, int err)
{
    RBL_LOG_FMT("read_error_termination fd %d read_state: %d \n", ctx->readfd, ctx->read_state);
    ctx->read_state = RD_STATE_ERROR;
    ctx->return_code = err;
    runloop_stream_deregister(ctx->reader);
}
void arm_fd_for_read(int kq, int readfd) 
{
    struct kevent change;
    EV_SET(&change, readfd, EVFILT_READ, EV_ADD|EV_ENABLE|EV_DISPATCH, 0,0,NULL);
    int res = kevent(kq, &change, 1, NULL, 0, NULL);
}
void* reader_thread_func_2(void* arg)
{
    int readfd = (int)arg;
    int kq = kqueue();
    arm_fd_for_read(kq, readfd);
    // struct kevent change;
    // EV_SET(&change, readfd, EVFILT_READ, EV_ADD|EV_ENABLE|EV_DISPATCH, 0,0,NULL);
    // int res = kevent(kq, &change, 1, NULL, 0, NULL);
    for(;;) {
        struct kevent events[1];
        int nev = kevent(kq, NULL, 0, events, 1, NULL);
        if(nev == -1) {
            perror("kevent");
        }
        for(int i = 0; i < nev; i++) {
            if(events[i].ident == readfd && events[i].filter == EVFILT_READ) {
                char rbuffer[265];
                size_t n = read(readfd, rbuffer, sizeof(rbuffer));
                if(n > 0) {
                    rbuffer[n] = '\0';
                    printf("buffer read : %s\n", rbuffer);
                    arm_fd_for_read(kq, readfd);
                } else if(n == 0) {
                    printf("End of file\n");
                } else {
                    perror("read");
                }
            } else {
                printf("spurious event %d\n", events[i].filter);
            }
        }
    }
    return NULL;
}
void* reader_thread_func_3(void* arg)
{
    int readfd = (int)arg;
    RunloopRef runloop = runloop_new();
    RunloopEventRef reader = runloop_stream_new(runloop, readfd);
    runloop_stream_arm_read(reader, read_callback2, reader);
    int kq = runloop->kqueue_fd;
    for(;;) {
        struct kevent events[1];
        int nev = kevent(kq, NULL, 0, events, 1, NULL);
        if(nev == -1) {
            perror("kevent");
        }
        for(int i = 0; i < nev; i++) {
            if(events[i].ident == readfd && events[i].filter == EVFILT_READ) {
                reader->handler(reader, events[i].filter, events[i].flags);
                // char rbuffer[265];
                // size_t n = read(readfd, rbuffer, sizeof(rbuffer));
                // if(n > 0) {
                //     rbuffer[n] = '\0';
                //     printf("buffer read : %s\n", rbuffer);
                //     arm_fd_for_read(kq, readfd);
                // } else if(n == 0) {
                //     printf("End of file\n");
                // } else {
                //     perror("read");
                // }
            } else {
                printf("spurious event %d\n", events[i].filter);
            }
        }
    }


}
void* reader_thread_func_1(void* arg)
{
    RunloopRef runloop_ref = runloop_new();
    ReaderTable* rdr = (ReaderTable*)arg;
    for(int i = 0; i < rdr->count; i++) {
        ReadCtx* ctx = &(rdr->ctx_table[i]);
        RBL_LOG_FMT("read_thread_func loop i: %d fd %d read_state: %d\n", i, ctx->readfd, ctx->read_state);

        rdr->ctx_table[i].reader = runloop_stream_new(runloop_ref, ctx->readfd);
        RunloopStreamRef sw = rdr->ctx_table[i].reader;
        // runloop_stream_register(sw);
        // runloop_stream_arm_read(sw, &read_callback, (void *) ctx);
        runloop_post(runloop_ref, postable_reader, ctx);
    }
    runloop_run(runloop_ref, 1000000);
    return NULL;
}
void* reader_thread_func(void*arg)
{
    reader_thread_func_3(arg);
}