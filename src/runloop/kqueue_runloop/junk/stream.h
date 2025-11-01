#ifndef C_HTTP_runloop_W_STREAM_H
#define C_HTTP_runloop_W_STREAM_H
#include "runloop.h"

/** \defgroup stream RunloopStream
 * @{
 * RunloopStreamRef is a special type of watcher designed to watch stream style file descriptors
 * (such a sockets and pipes) to detect when such an fd is ready to perform a read operation or ready
 * to perform a write operation without BLOCKING. This type of watcher is intended for situations
 * where a single thread may be performing both read and write operations on the same fd and on
 * multiple file descriptors.
 */

RunloopStreamRef runloop_stream_new(RunloopRef runloop, int fd);
void runloop_stream_free(RunloopStreamRef athis);

void runloop_stream_init(RunloopStreamRef athis, RunloopRef runloop, int fd);
void runloop_stream_deinit(RunloopStreamRef athis);

void runloop_stream_register(RunloopStreamRef athis);
void runloop_stream_deregister(RunloopStreamRef athis);

void runloop_stream_arm_both(RunloopStreamRef athis,
                             PostableFunction read_postable_cb, void* read_arg,
                             PostableFunction write_postable_cb, void* write_arg);

void runloop_stream_arm_read(RunloopStreamRef athis, PostableFunction postable_callback, void* arg);
void runloop_stream_disarm_read(RunloopStreamRef athis);

void runloop_stream_arm_write(RunloopStreamRef athis, PostableFunction postable_callback, void* arg);
void runloop_stream_disarm_write(RunloopStreamRef athis);

void runloop_stream_verify(RunloopStreamRef r);

RunloopRef runloop_stream_get_runloop(RunloopStreamRef athis);
int runloop_stream_get_fd(RunloopStreamRef this);
void runloop_stream_checktag(RunloopStreamRef athis);

/**
 * Async io is a more convenient interface for reading and writing data to fd's like sockets.
 *
 * It is a proactor interface rather than the reactor provided by the runloop_stream
 * interface above
 */
AsioStreamRef asio_stream_new(RunloopRef runloop_ref, int socket);
void asio_stream_free(AsioStreamRef this);
void asio_stream_init(AsioStreamRef this, RunloopRef runloop_ref, int fd);
void asio_stream_deinit(AsioStreamRef cref);
void asio_stream_read(AsioStreamRef stream_ref, void* buffer, long max_length, AsioReadcallback cb, void*  arg);
void asio_stream_write(AsioStreamRef stream_ref, void* buffer, long length, AsioWritecallback cb, void*  arg);
void asio_stream_close(AsioStreamRef cref);
RunloopRef asio_stream_get_runloop(AsioStreamRef asio_stream_ref);
int asio_stream_get_fd(AsioStreamRef asio_stream_ref);
RunloopStreamRef asio_stream_get_runloop_stream(AsioStreamRef asio_stream_ref);

/** @} */
#endif //C_HTTP_runloop_W_STREAM_H
