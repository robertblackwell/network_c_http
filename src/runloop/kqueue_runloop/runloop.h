#ifndef C_HTTP_KQ_RUNLOOP_H
#define C_HTTP_KQ_RUNLOOP_H

#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Types -= forward declares
///////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct Runloop_s Runloop, *RunloopRef;
typedef struct  RunloopEvent_s RunloopEvent, *RunloopEventRef,
                RunloopTimer, * RunloopTimerRef,
                RunloopListener, *RunloopListenerRef,
                RunloopStream, *RunloopStreamRef,
                RunloopUserEvent, *RunloopUserEventRef;
typedef struct RunloopWatcherBase_s RunloopWatcherBase, *RunloopWatcherBaseRef;   
typedef struct EventQueue_s EventQueue, * EventQueueRef;
typedef struct InterthreadQueue_s InterthreadQueue, *InterthreadQueueRef;
typedef struct RunloopQueueWatcher_s RunloopQueueWatcher, *RunloopQueueWatcherRef;
/**
 * PostableFunction defines the call signature of functions that can be added to a runloops queue of
 * functions to be called. As such they represent the next step in an ongoing computation of a lightweight
 * "thread".
 */
typedef void (*PostableFunction) (RunloopRef runloop_ref, void* arg);
// typedef void(*AsioReadcallback)(void* arg, long length, int error_number);
// typedef void(*AsioWritecallback)(void* arg, long length, int error_number);
typedef void(*AcceptCallback)(void* arg, int accepted_fd, int errno);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Functors - not sure why it this promonent
///////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct Functor_s
{
    PostableFunction f;
    void *arg;
} Functor, *FunctorRef;

typedef uint64_t EventMask, RunloopTimerEvent;
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Runloop interface
///////////////////////////////////////////////////////////////////////////////////////////////////////////
RunloopRef runloop_get_threads_runloop();
RunloopRef runloop_new(void);
void runloop_free(RunloopRef athis);
void runloop_init(RunloopRef athis);
void runloop_deinit(RunloopRef athis);
void runloop_close(RunloopRef athis);
int  runloop_register(RunloopRef athis, int fd, uint32_t interest, RunloopWatcherBaseRef wref);
int  runloop_deregister(RunloopRef athis, int fd);
int  runloop_reregister(RunloopRef athis, int fd, uint32_t interest, RunloopWatcherBaseRef wref);
int  runloop_run(RunloopRef athis, time_t timeout);
void runloop_post(RunloopRef athis, PostableFunction cb, void* arg);
void runloop_delete(RunloopRef athis, int fd);
void runloop_verify(RunloopRef r);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Timers
///////////////////////////////////////////////////////////////////////////////////////////////////////////
RunloopTimerRef runloop_timer_new(RunloopRef runloop_ref);
void runloop_timer_init(RunloopTimerRef lrevent, RunloopRef runloop);
void runloop_timer_free(RunloopTimerRef lrevent);
void runloop_timer_register(RunloopTimerRef lrevent, PostableFunction cb, void* ctx, uint64_t interval_ms, bool repeating);
void runloop_timer_update(RunloopTimerRef lrevent, uint64_t interval_ms, bool repeating);
void runloop_timer_disarm(RunloopTimerRef lrevent);
void runloop_timer_rearm_old(RunloopTimerRef lrevent, PostableFunction cb, void* ctx, uint64_t interval_ms, bool repeating);
void runloop_timer_rearm(RunloopTimerRef lrevent);
void runloop_timer_deregister(RunloopTimerRef lrevent);
RunloopRef runloop_timer_get_runloop(RunloopTimerRef lrevent);
RunloopTimerRef runloop_timer_set(RunloopRef rl, PostableFunction cb, void* ctx, uint64_t interval_ms, bool repeating);
void runloop_timer_clear(RunloopRef rl, RunloopTimerRef lrevent);
void runloop_timer_checktag(RunloopTimerRef lrevent);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Runloop Lsitener
///////////////////////////////////////////////////////////////////////////////////////////////////////////

RunloopListenerRef runloop_listener_new(RunloopRef runloop, int fd);
void runloop_listener_free(RunloopListenerRef lrevent);
void runloop_listener_init(RunloopListenerRef lrevent, RunloopRef runloop, int fd);
void runloop_listener_deinit(RunloopListenerRef lrevent);
void runloop_listener_register(RunloopListenerRef lrevent, PostableFunction postable, void* postable_arg);
void runloop_listener_deregister(RunloopListenerRef lrevent);
void runloop_listener_arm(RunloopListenerRef lrevent, PostableFunction postable, void* postable_arg);
void runloop_listener_disarm(RunloopListenerRef lrevent);
void runloop_listener_verify(RunloopListenerRef lrevent);
RunloopRef runloop_listener_get_runloop(RunloopListenerRef lrevent);
int runloop_listener_get_fd(RunloopListenerRef lrevent);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// RunloopStream
///////////////////////////////////////////////////////////////////////////////////////////////////////////
RunloopStreamRef runloop_stream_new(RunloopRef runloop, int fd);
void runloop_stream_init(RunloopStreamRef lrevent, RunloopRef runloop, int fd);
void runloop_stream_free(RunloopStreamRef lrevent);
void runloop_stream_deinit(RunloopStreamRef lrevent);
void runloop_stream_register(RunloopStreamRef lrevent);
void runloop_stream_deregister(RunloopStreamRef lrevent);
void runloop_stream_arm_both(RunloopStreamRef lrevent,
                             PostableFunction read_postable_cb, void* read_arg,
                             PostableFunction write_postable_cb, void* write_arg);

void runloop_stream_arm_read(RunloopStreamRef lrevent, PostableFunction postable_callback, void* arg);
void runloop_stream_disarm_read(RunloopStreamRef lrevent);
void runloop_stream_arm_write(RunloopStreamRef lrevent, PostableFunction postable_callback, void* arg);
void runloop_stream_disarm_write(RunloopStreamRef lrevent);
void runloop_stream_verify(RunloopStreamRef r);
RunloopRef runloop_stream_get_runloop(RunloopStreamRef lrevent);
int runloop_stream_get_fd(RunloopStreamRef this);
void runloop_stream_checktag(RunloopStreamRef lrevent);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// User Event
///////////////////////////////////////////////////////////////////////////////////////////////////////////
/* *
 * kqueue provides a facility to create and wait on an event source that is not attached to any fd/file/pipe/device
 * and to "fire" such events explicitly.
 * One of the variants of the RunloopEvent struct and related functions use the kqueue facility to provide a
 * generalized mechanism for creating custom events that can be fired and notified
 * using the standard kqueue feature.
 */
RunloopUserEventRef runloop_user_event_new(RunloopRef runloop);
void runloop_user_event_init(RunloopUserEventRef athis, RunloopRef runloop);
void runloop_user_event_free(RunloopUserEventRef athis);
void runloop_user_event_register(RunloopUserEventRef athis);
void runloop_user_event_change_watch(RunloopUserEventRef athis, PostableFunction postable, void* arg, uint64_t watch_what);
void runloop_user_event_arm(RunloopUserEventRef athis, PostableFunction postable, void* arg);
void runloop_user_event_disarm(RunloopUserEventRef athis);
void runloop_user_event_fire(RunloopUserEventRef athis);
void runloop_user_event_clear_one_event(RunloopUserEventRef athis);
void runloop_user_event_clear_all_events(RunloopUserEventRef athis);
void runloop_user_event_deregister(RunloopUserEventRef athis);
void runloop_user_event_verify(RunloopUserEventRef r);
RunloopRef runloop_user_event_get_reactor(RunloopUserEventRef athis);
int runloop_user_event_get_fd(RunloopUserEventRef this);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// User Event Queue
///////////////////////////////////////////////////////////////////////////////////////////////////////////
EventQueueRef runloop_event_queue_new(RunloopRef runloop, EventQueueRef this);
void runloop_event_queue_init(RunloopRef runloop, EventQueueRef this);
void runloop_event_queue_deinit(EventQueueRef this);
void  runloop_event_queue_free(EventQueueRef athis);
void  runloop_event_queue_add(EventQueueRef athis, Functor item);
Functor runloop_event_queue_remove(EventQueueRef athis);
int   runloop_event_queue_readfd(EventQueueRef athis);
RunloopRef runloop_event_queue_get_runloop(EventQueueRef athis);

RunloopQueueWatcherRef runloop_queue_watcher_new(RunloopRef runloop, EventQueueRef qref);
void runloop_queue_watcher_init(RunloopQueueWatcherRef qw, RunloopRef runloop, EventQueueRef qref);
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
#include "asio.h"
#endif