#ifndef C_HTTP_EPOLL_RUNLOOP_H
#define C_HTTP_EPOLL_RUNLOOP_H

#include <stdint.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Types -= forward declares
///////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct Runloop_s Runloop, *RunloopRef;
typedef struct RunloopWatcherBase_s RunloopWatcherBase, *RunloopWatcherBaseRef;
typedef struct RunloopTimer_s RunloopTimer,  *RunloopTimerRef;  
typedef struct RunloopListener_s RunloopListener, * RunloopListenerRef;  
typedef struct RunloopStream_s RunloopStream, *RunloopStreamRef;         
typedef struct RunloopEventfd_s RunloopEventfd, *RunloopEventfdRef;      
typedef struct EventfdQueue_s EventfdQueue, * EventfdQueueRef;
typedef struct InterthreadQueue_s InterthreadQueue, *InterthreadQueueRef;
typedef struct RunloopQueueWatcher_s RunloopQueueWatcher, *RunloopQueueWatcherRef; 
typedef struct AsioStream_s AsioStream, *AsioStreamRef;
typedef struct AsioListener_s AsioListener, *AsioListenerRef;
/**
 * PostableFunction defines the call signature of functions that can be added to a runloops queue of
 * functions to be called. As such they represent the next step in an ongoing computation of a lightweight
 * "thread".
 */
typedef void (*PostableFunction) (RunloopRef runloop_ref, void* arg);
typedef void(*AsioReadcallback)(void* arg, long length, int error_number);
typedef void(*AsioWritecallback)(void* arg, long length, int error_number);
typedef void(*AcceptCallback)(void* arg, int accepted_fd, int errno);


///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Functors - not sure why it this promonent
///////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct Functor_s
{
//    RunloopWatcherBaseRef wref; // this is borrowed do not free
    PostableFunction f;
    void *arg;
} Functor, *FunctorRef;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Runloop interface
///////////////////////////////////////////////////////////////////////////////////////////////////////////
RunloopRef runloop_get_threads_reactor();
RunloopRef runloop_new(void);
void       runloop_free(RunloopRef athis);
void       runloop_init(RunloopRef athis);
void       runloop_deinit(RunloopRef athis);
void       runloop_close(RunloopRef athis);
int        runloop_register(RunloopRef athis, int fd, uint32_t interest, RunloopWatcherBaseRef wref);
int        runloop_deregister(RunloopRef athis, int fd);
int        runloop_reregister(RunloopRef athis, int fd, uint32_t interest, RunloopWatcherBaseRef wref);
int        runloop_run(RunloopRef athis, time_t timeout);
void       runloop_post(RunloopRef athis, PostableFunction cb, void* arg);
void       runloop_delete(RunloopRef athis, int fd);
void       runloop_verify(RunloopRef r);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Timers
///////////////////////////////////////////////////////////////////////////////////////////////////////////
RunloopTimerRef runloop_timer_new(RunloopRef runloop_ref);
void runloop_timer_init(RunloopTimerRef this, RunloopRef runloop);
void runloop_timer_free(RunloopTimerRef athis);
void runloop_timer_register(RunloopTimerRef athis, PostableFunction cb, void* ctx, uint64_t interval_ms, bool repeating);
void runloop_timer_update(RunloopTimerRef athis, uint64_t interval_ms, bool repeating);
void runloop_timer_disarm(RunloopTimerRef athis);
void runloop_timer_rearm_old(RunloopTimerRef athis, PostableFunction cb, void* ctx, uint64_t interval_ms, bool repeating);
void runloop_timer_rearm(RunloopTimerRef athis);
void runloop_timer_deregister(RunloopTimerRef athis);

RunloopRef runloop_timer_get_runloop(RunloopTimerRef athis);
int runloop_timer_get_fd(RunloopTimerRef timer);

RunloopTimerRef runloop_timer_set(RunloopRef rl, PostableFunction cb, void* ctx, uint64_t interval_ms, bool repeating);
void runloop_timer_clear(RunloopRef rl, RunloopTimerRef timerref);
void runloop_timer_checktag(RunloopTimerRef athis);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Runloop Lsitener
///////////////////////////////////////////////////////////////////////////////////////////////////////////

RunloopListenerRef runloop_listener_new(RunloopRef runloop, int fd);
void runloop_listener_free(RunloopListenerRef athis);
void runloop_listener_init(RunloopListenerRef athis, RunloopRef runloop, int fd);
void runloop_listener_deinit(RunloopListenerRef athis);
void runloop_listener_register(RunloopListenerRef athis, PostableFunction postable, void* postable_arg);
void runloop_listener_deregister(RunloopListenerRef athis);
void runloop_listener_arm(RunloopListenerRef athis, PostableFunction postable, void* postable_arg);
void runloop_listener_disarm(RunloopListenerRef athis);
void runloop_listener_verify(RunloopListenerRef r);
RunloopRef runloop_listener_get_runloop(RunloopListenerRef athis);
int runloop_listener_get_fd(RunloopListenerRef this);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// RunloopStream
///////////////////////////////////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// User Event
///////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * epoll provides a facility to create a file descriptor that is not attached to any file/pipe/device
 * and to "fire" events on that file descriptor that can be waited for using the epoll call.
 * This facility provides a mechanism to create and wait on arbitary event sources.
 * 
 */
RunloopEventfdRef runloop_user_event_new(RunloopRef runloop);
void runloop_user_event_init(RunloopEventfdRef athis, RunloopRef runloop);
void runloop_user_event_free(RunloopEventfdRef athis);
void runloop_user_event_register(RunloopEventfdRef athis);
void runloop_user_event_change_watch(RunloopEventfdRef athis, PostableFunction postable, void* arg, uint64_t watch_what);
void runloop_user_event_arm(RunloopEventfdRef athis, PostableFunction postable, void* arg);
void runloop_user_event_disarm(RunloopEventfdRef athis);
void runloop_user_event_fire(RunloopEventfdRef athis);
void runloop_user_event_clear_one_event(RunloopEventfdRef athis);
void runloop_user_event_clear_all_events(RunloopEventfdRef athis);
void runloop_user_event_deregister(RunloopEventfdRef athis);
void runloop_user_event_verify(RunloopEventfdRef r);
RunloopRef runloop_user_event_get_reactor(RunloopEventfdRef athis);
int runloop_user_event_get_fd(RunloopEventfdRef this);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// User Event Queue
///////////////////////////////////////////////////////////////////////////////////////////////////////////
EventfdQueueRef runloop_user_event_queue_new();
void user_event_queue_init(RunloopRef runloop, EventfdQueueRef this);
void user_event_queue_deinit(EventfdQueueRef this);
void  runloop_user_event_queue_free(EventfdQueueRef athis);
void  runloop_user_event_queue_add(EventfdQueueRef athis, Functor item);
Functor runloop_user_event_queue_remove(EventfdQueueRef athis);
int   runloop_user_event_queue_readfd(EventfdQueueRef athis);
RunloopRef runloop_user_event_queue_get_runloop(EventfdQueueRef athis);

RunloopQueueWatcherRef runloop_queue_watcher_new(RunloopRef runloop, EventfdQueueRef qref);
void runloop_queue_watcher_init(RunloopQueueWatcherRef qw, RunloopRef runloop, EventfdQueueRef qref);
void runloop_queue_watcher_deinit(RunloopQueueWatcherRef qw);
void runloop_queue_watcher_free(RunloopQueueWatcherRef this);

typedef void(*QueueWatcherReadCallbackFunction)(void* context_ptr, Functor item, int status);
void runloop_queue_watcher_async_read(RunloopQueueWatcherRef queue_watcher_ref, QueueWatcherReadCallbackFunction callback, void* context_ptr);

void runloop_queue_watcher_register(RunloopQueueWatcherRef athis, PostableFunction postable_cb, void* postable_arg);
void runloop_queue_watcher_change_watch(RunloopQueueWatcherRef athis, PostableFunction postable_cb, void* arg, uint64_t watch_what);
void runloop_queue_watcher_deregister(RunloopQueueWatcherRef athis);
void runloop_queue_watcher_verify(RunloopQueueWatcherRef r);
RunloopRef runloop_queue_watcher_get_reactor(RunloopQueueWatcherRef athis);
int runloop_queue_watcher_get_fd(RunloopQueueWatcherRef this);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Base event
///////////////////////////////////////////////////////////////////////////////////////////////////////////
RunloopRef runloop_watcher_base_get_runloop(RunloopWatcherBaseRef athis);
int        runloop_watcher_base_get_fd(RunloopWatcherBaseRef this);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Async io is a more convenient interface for reading and writing data to fd's like sockets.
 *
 * It is a proactor interface rather than the reactor provided by the runloop_stream
 * interface above
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////
AsioStreamRef asio_stream_new(RunloopRef runloop_ref, int socket);
void asio_stream_free(AsioStreamRef this);
void asio_stream_init(AsioStreamRef this, RunloopRef runloop_ref, int fd);
void asio_stream_deinit(AsioStreamRef cref);
void asio_stream_read(AsioStreamRef stream_ref, void* buffer, long max_length, AsioReadcallback cb, void*  arg);
void asio_stream_write(AsioStreamRef stream_ref, void* buffer, long length, AsioWritecallback cb, void*  arg);
void asio_stream_close(AsioStreamRef cref);
RunloopStreamRef asio_stream_get_runloop_stream(AsioStreamRef asio_stream_ref);
RunloopRef asio_stream_get_runloop(AsioStreamRef asio_stream_ref);
int asio_stream_get_fd(AsioStreamRef asio_stream_ref);

AsioListenerRef asio_listener_new_from_port_host(RunloopRef rlref, int port, const char* host);
AsioListenerRef asio_listener_new(RunloopRef rlref, int socket_fd);
void asio_listener_init(AsioListenerRef this, RunloopRef rl, int socket_fd);
void asio_listen_init_from_port_host(AsioListenerRef this, int port , const char* host);
void asio_listener_deinit(AsioListenerRef this);
void asio_listener_free(AsioListenerRef this);
void asio_accept(AsioListenerRef alistener_ref, void(on_accept_cb)(void* arg, int accepted_fd, int error), void* arg);
RunloopListenerRef asio_listener_get_runloop_listener(AsioListenerRef asio_listener_ref);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Type safe - these macros provides functions to assert - that is crash if not - the types:
//
// -    RunloopRef
// -    specific subtypes of RunloopEventRef
//      - runloop_listener_verify(p)
//      - runloop_signal_verify(p)
//      - runloop_stream_verify(p)
//      - runloop_timer_verify(p)
//      - runloop_user_event_queue_verify(p)
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#define RUNLOOP_VERIFY(p) runloop_verify(p, __FILE__, __LINE__);
#define RUNLOOP_LISTENER_VERIFY(p) runloop_listener_verify(p, __FILE__, __LINE__);
#define RUNLOOP_SIGNAL_VERIFY(p) runloop_signal_verify(p, __FILE__, __LINE__);
#define RUNLOOP_STREAM_VERIFY(p) runloop_stream_verify(p, __FILE__, __LINE__);
#define RUNLOOP_TIMER_VERIFY(p) runloop_timer_verify(p, __FILE__, __LINE__);
#define RUNLOOP_USER_EVENT_VERIFY(p) runloop_user_event_verify(p, __FILE__, __LINE__);
#define RUNLOOP_USER_EVENT_QUEUE_VERIFY(p) runloop_user_event_queue_verify(p, __FILE__, __LINE__);
#define RUNLOOP_VERIFY(p) runloop_verify(p, __FILE__, __LINE__);

#include "rl_checktag.h"
#endif