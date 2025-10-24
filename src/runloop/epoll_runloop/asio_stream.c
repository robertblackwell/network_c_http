//#define RBL_LOG_ENABLED
//#define RBL_LOG_ALLOW_GLOBAL
#include "runloop_internal.h"
#include <rbl/check_tag.h>
#include <rbl/logger.h>
#include <rbl/macros.h>
#include <common/socket_functions.h>
#include "rl_internal.h"
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>

#define STREAM_LEVEL_TRIGGERED
#ifndef STREAM_LEVEL_TRIGGERED
    #define STREAM_EDGE_TRIGGERED
#endif

#define READ_STATE_IDLE 11
#define READ_STATE_ACTIVE 12
#define READ_STATE_EAGAIN 13

#define WRITE_STATE_IDLE 21
#define WRITE_STATE_ACTIVE 22
#define WRITE_STATE_EAGAIN 22

static void try_read(AsioStreamRef cref);
static void try_write(AsioStreamRef cref);
static void read_eagain(AsioStreamRef cref);
static void write_eagain(AsioStreamRef cref);
static void epollin_postable_cb(RunloopRef rl, void* cref_arg);
static void epollout_postable_cb(RunloopRef rl, void* cref_arg);

AsioStreamRef asio_stream_new(RunloopRef reactor_ref, int socket)
{
    AsioStreamRef this = malloc(sizeof(AsioStream));
    asio_stream_init(this, reactor_ref, socket);
    return this;
}
void asio_stream_init(AsioStreamRef this, RunloopRef runloop_ref, int socket)
{
    RBL_ASSERT((this != NULL), "")
    RBL_SET_TAG(AsioStream_TAG, this)
    RBL_SET_END_TAG(AsioStream_TAG, this)
    RBL_CHECK_TAG(AsioStream_TAG, this)
    RBL_CHECK_END_TAG(AsioStream_TAG, this)
    RBL_LOG_FMT("AsioStream socket: %p, fd: %d", this, socket)
//    this->runloop_ref = reactor_ref;
    this->fd = socket;
    this->runloop_stream_ref = runloop_stream_new(runloop_ref, socket);
    this->read_state = READ_STATE_IDLE;
    this->read_callback = NULL;
    this->read_callback_arg = NULL;

    this->write_state = WRITE_STATE_IDLE;
    this->read_callback = NULL;
    this->read_callback_arg = NULL;

    this->read_buffer_size = 1000000;
    socket_set_non_blocking(socket);
    runloop_stream_register(this->runloop_stream_ref);
#ifdef STREAM_EDGE_TRIGGERED
    runloop_stream_arm_both(this->runloop_stream_ref,
                            epollin_postable_cb, this,
                            epollout_postable_cb, this);
#endif
}
void asio_stream_destroy(AsioStreamRef this)
{
    RBL_ASSERT((this != NULL), "")
    RBL_CHECK_TAG(AsioStream_TAG, this)
    RBL_CHECK_END_TAG(AsioStream_TAG, this)
    int fd = this->runloop_stream_ref->fd;
    int fd2 = this->fd;
    assert(fd == fd2);

    if(fd > 0) {
        asio_stream_close(this);
    }
    free(this->runloop_stream_ref);
    this->runloop_stream_ref = NULL;
    RBL_LOG_FMT("asio_stream_free close socket: %d", fd)
    RBL_SET_TAG("xxxxxxx", this) // corrupt the tag
    RBL_INVALIDATE_TAG(this)
}
void asio_stream_close(AsioStreamRef cref)
{
    RBL_ASSERT((cref != NULL), "")
    RBL_CHECK_TAG(AsioStream_TAG, cref)
    RBL_CHECK_END_TAG(AsioStream_TAG, cref)
    RBL_ASSERT((cref->fd > 0), "socket should be positive");
    /**
     * @TODO - is this right
    runloop_stream_deregister(cref->runloop_stream_ref);
    */
    close(cref->fd);
    cref->fd = -1;
    cref->runloop_stream_ref->fd = -1;
}
void asio_stream_free(AsioStreamRef this)
{
    RBL_ASSERT((this != NULL), "")
    RBL_CHECK_TAG(AsioStream_TAG, this)
    RBL_CHECK_END_TAG(AsioStream_TAG, this)
    asio_stream_destroy(this);
    free(this);
}

void asio_stream_read(AsioStreamRef connection_ref, void* buffer, long max_length, AsioReadcallback cb, void*  arg)
{
    RBL_ASSERT((connection_ref != NULL), "")
    RBL_CHECK_TAG(AsioStream_TAG, connection_ref)
    RBL_CHECK_END_TAG(AsioStream_TAG, connection_ref)
    RBL_ASSERT((connection_ref->read_state == READ_STATE_IDLE), "can only call asio_stream_read once");
    RBL_ASSERT((buffer != NULL), "read buffer is NULL");
    RBL_ASSERT((max_length > 0), "read buffer max length <= 0");
    connection_ref->read_buffer = buffer;
    connection_ref->read_buffer_size = max_length;
    connection_ref->read_callback = cb;
    connection_ref->read_callback_arg = arg;
    connection_ref->read_state = READ_STATE_ACTIVE;
    try_read(connection_ref);
}
void asio_stream_write(AsioStreamRef connection_ref, void* buffer, long length, AsioWritecallback cb, void*  arg)
{
    RBL_ASSERT((connection_ref != NULL), "")
    RBL_CHECK_TAG(AsioStream_TAG, connection_ref)
    RBL_CHECK_END_TAG(AsioStream_TAG, connection_ref)
    RBL_ASSERT((connection_ref->write_state == WRITE_STATE_IDLE), "a write is already active");
    RBL_ASSERT((buffer != NULL), "write buffer is NULL");
    RBL_ASSERT((length > 0), "write buffer max length <= 0");
    connection_ref->write_buffer = buffer;
    connection_ref->write_buffer_size = length;
    connection_ref->write_callback = cb;
    connection_ref->write_callback_arg = arg;
    connection_ref->write_state = WRITE_STATE_ACTIVE;
    try_write(connection_ref);
}
RunloopStreamRef asio_stream_get_runloop_stream(AsioStreamRef asio_stream_ref)
{
    return asio_stream_ref->runloop_stream_ref;
}
RunloopRef asio_stream_get_runloop(AsioStreamRef asio_stream_ref)
{
    return asio_stream_ref->runloop_stream_ref->runloop;
}
int asio_stream_get_fd(AsioStreamRef athis)
{
    return athis->runloop_stream_ref->fd;
}
static void try_read(AsioStreamRef cref)
{
    RBL_ASSERT((cref != NULL), "")
    RBL_CHECK_TAG(AsioStream_TAG, cref)
    RBL_CHECK_END_TAG(AsioStream_TAG, cref)
    long nread = read(cref->fd, cref->read_buffer, cref->read_buffer_size);
    RBL_LOG_FMT("asio_stream %p nread: %ld", cref, nread)
    int saved_errno = errno;
    if(nread > 0) {
        cref->read_state = READ_STATE_IDLE;
        cref->read_callback(cref->read_callback_arg, nread, 0);
        cref->read_callback = NULL;
    } else if(nread == 0) {
        // the other end closed the connection
        cref->read_callback(cref->read_callback_arg, 0, 0);
        cref->read_callback = NULL;
    } else if((saved_errno == EAGAIN) || (saved_errno == EWOULDBLOCK)) {
        // posix uses different values for EAGAIN and EWOULDBLOCK but on linux they are the same
        read_eagain(cref);
    } else {
        cref->read_callback(cref->read_callback_arg, 0, saved_errno);
        cref->read_callback = NULL;
    }
}
static void read_eagain(AsioStreamRef cref)
{
    RBL_ASSERT((cref != NULL), "")
    RBL_CHECK_TAG(AsioStream_TAG, cref)
    RBL_CHECK_END_TAG(AsioStream_TAG, cref)
    RBL_LOG_FMT("asio_stream %p", cref);
    cref->read_state = READ_STATE_EAGAIN;
#ifdef STREAM_LEVEL_TRIGGERED
    RBL_LOG_FMT("read_eagain arm read fd: %d\n", cref->runloop_stream_ref->fd);
    runloop_stream_arm_read(cref->runloop_stream_ref, epollin_postable_cb, cref);
#endif
}
static void epollin_postable_cb(RunloopRef rl, void* cref_arg)
{
    AsioStreamRef cref = cref_arg;
    RBL_ASSERT((cref != NULL), "")
    RBL_LOG_FMT("asio_sttream %p", cref)
    RBL_CHECK_TAG(AsioStream_TAG, cref)
    RBL_CHECK_END_TAG(AsioStream_TAG, cref)
    if(cref->read_state == READ_STATE_EAGAIN) {
        // disarm read
        if(cref->read_callback == NULL) {
            cref->read_state = READ_STATE_IDLE;
        } else {
            RBL_LOG_FMT("epollin_postable_cb disram read fd: %d\n", cref->runloop_stream_ref->fd);
            runloop_stream_disarm_read(cref->runloop_stream_ref);
            try_read(cref);
        }
    }
}
static void try_write(AsioStreamRef cref)
{
    RBL_ASSERT((cref != NULL), "")
    RBL_CHECK_TAG(AsioStream_TAG, cref)
    RBL_CHECK_END_TAG(AsioStream_TAG, cref)
    long n = write(cref->fd, cref->write_buffer, cref->write_buffer_size);
    RBL_LOG_FMT("asio_stream %p fd: %d nwrite: %ld ", cref, cref->fd, n)
    int saved_errno = errno;
    if(n > 0) {
        cref->write_state = WRITE_STATE_IDLE;
        cref->write_callback(cref->write_callback_arg, n, 0);
        cref->write_callback = NULL;
    } else if(n == 0) {
        // the other end closed the connection - end-of-file
        cref->write_callback(cref->read_callback_arg, 0, 0);
        cref->write_callback = NULL;
    } else if((saved_errno == EAGAIN) || (saved_errno == EWOULDBLOCK)) {
        // posix uses different values for EAGAIN and EWOULDBLOCK but on linux they are the same
        write_eagain(cref);
    } else {
        // io error
        cref->write_callback(cref->read_callback_arg, 0, saved_errno);
        cref->write_callback = NULL;
    }
}

static void write_eagain(AsioStreamRef cref)
{
    RBL_ASSERT((cref != NULL), "")
    RBL_CHECK_TAG(AsioStream_TAG, cref)
    RBL_CHECK_END_TAG(AsioStream_TAG, cref)
    cref->read_state = READ_STATE_EAGAIN;
#ifdef STREAM_LEVEL_TRIGGERED
    runloop_stream_arm_write(cref->runloop_stream_ref, epollout_postable_cb, cref);
#endif
    // arm write
}
static void epollout_postable_cb(RunloopRef rl, void* cref_arg)
{
    AsioStreamRef cref = cref_arg;
    RBL_ASSERT((cref != NULL), "")
    RBL_CHECK_TAG(AsioStream_TAG, cref)
    RBL_CHECK_END_TAG(AsioStream_TAG, cref)
    if(cref->write_state == WRITE_STATE_EAGAIN) {
        // disarm write
        if(cref->write_callback == NULL) {
            cref->write_state = WRITE_STATE_IDLE;
        } else {
            try_read(cref);
        }
    }
}
