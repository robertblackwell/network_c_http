#ifndef C_HTTP_RUNLOOP_API_H
#define C_HTTP_RUNLOOP_API_H

#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Types -= forward declares
///////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct Runloop_s Runloop, *RunloopRef;
typedef struct RunloopEventBase_s RunloopWatcherBase, *RunloopEventBaseRef;
typedef struct RunloopTimer_s RunloopTimer, *RunloopTimerRef;
typedef struct RunloopListener_s RunloopListener, *RunloopListenerRef;
typedef struct RunloopStream_s RunloopStream, *RunloopStreamRef;
typedef struct RunloopUserEvent_s RunloopUserEvent, *RunloopUserEventRef;
typedef struct RunloopSignal_s RunloopSignal, *RunloopSignalRef;
typedef struct UserEventQueue_s UserEventQueue, * UserEventQueueRef;
// typedef struct InterthreadQueue_s InterthreadQueue, *InterthreadQueueRef;
// typedef struct RunloopQueueWatcher_s RunloopQueueWatcher, *RunloopQueueWatcherRef;
/**
 * PostableFunction defines the call signature of functions that can be added to a runloops queue of
 * functions to be called. As such they represent the next step in an ongoing computation of a lightweight
 * "thread".
 */
typedef void (*PostableFunction) (RunloopRef runloop_ref, void* arg);
typedef void (*UserEventCallback) (RunloopRef runloop_ref, void* arg);
typedef void (*UserEventQueueCallback) (RunloopRef runloop_ref, void* arg);
// typedef void(*AsioReadcallback)(void* arg, long length, int error_number);
// typedef void(*AsioWritecallback)(void* arg, long length, int error_number);
typedef void(*AcceptCallback)(void* arg, int accepted_fd, int errno);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Functors - not sure why it this prominent
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
typedef struct RunloopConfig_s
{
    /**
     * A runloop allocates event sources such as RunloopTimer and RunloopStream from an object pool
     * of fixed size set at creation time. max_nbr_events sets the size of that object pool.
     */
    int max_nbr_events;
    /**
     * A runloop maintains a list of callback functions (a run list) waiting to be called. This list has a maximum size
     * set at startup time and list items are added and removed by value. This means that the run list can operate
     * without allocation and deallocation of dynamic memory.
     *
     * To set the size of the run list the runloop requires a parameter that tells it the max number of callbacks
     * that could be outstanding at the same time for each event source. So for example if the runloop is supporting
     * a traditional http server, that processes a single request at a time, and does not apply any timeouts,
     * this number is probably 1. For a full duplex TCP based protocol that both sends and receives messages
     * at the same time and applies a timeout to the receives side this number might be 3.
     */
    int max_simultaneous_callbacks_per_event;
}RunloopConfig ;
RunloopRef runloop_new(void);
RunloopRef runloop_new_with_config(RunloopConfig* config);
void runloop_free(RunloopRef athis);
void runloop_close(RunloopRef athis);
int  runloop_run(RunloopRef athis, time_t timeout);
void runloop_post(RunloopRef athis, PostableFunction cb, void* arg);
void runloop_verify(RunloopRef r);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Timers
///////////////////////////////////////////////////////////////////////////////////////////////////////////
RunloopTimerRef runloop_timer_new(RunloopRef runloop_ref);
void runloop_timer_init(RunloopTimerRef timer, RunloopRef runloop);
void runloop_timer_free(RunloopTimerRef timer);
void runloop_timer_register(RunloopTimerRef timer, PostableFunction cb, void* ctx, uint64_t interval_ms, bool repeating);
#if defined(__APPLE__)
void runloop_timer_update(RunloopTimerRef timer, uint64_t interval_ms, bool repeating);
#elif defined(__linux__)
void runloop_timer_update(RunloopTimerRef timer, PostableFunction cb, void* ctx, uint64_t interval_ms, bool repeating);
#endif
void runloop_timer_disarm(RunloopTimerRef timer);
void runloop_timer_rearm_old(RunloopTimerRef timer, PostableFunction cb, void* ctx, uint64_t interval_ms, bool repeating);
void runloop_timer_rearm(RunloopTimerRef timer);
void runloop_timer_deregister(RunloopTimerRef timer);
RunloopRef runloop_timer_get_runloop(RunloopTimerRef timer);
RunloopTimerRef runloop_timer_set(RunloopRef rl, PostableFunction cb, void* ctx, uint64_t interval_ms, bool repeating);
void runloop_timer_clear(RunloopRef rl, RunloopTimerRef timer);
void runloop_timer_checktag(RunloopTimerRef timer);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Runloop Lsitener
///////////////////////////////////////////////////////////////////////////////////////////////////////////

RunloopListenerRef runloop_listener_new(RunloopRef runloop, int fd);
void runloop_listener_free(RunloopListenerRef listener);
void runloop_listener_init(RunloopListenerRef listener, RunloopRef runloop, int fd);
void runloop_listener_deinit(RunloopListenerRef listener);
void runloop_listener_register(RunloopListenerRef listener, PostableFunction postable, void* postable_arg);
void runloop_listener_deregister(RunloopListenerRef listener);
void runloop_listener_arm(RunloopListenerRef listener, PostableFunction postable, void* postable_arg);
void runloop_listener_disarm(RunloopListenerRef listener);
void runloop_listener_verify(RunloopListenerRef listener);
RunloopRef runloop_listener_get_runloop(RunloopListenerRef listener);
int runloop_listener_get_fd(RunloopListenerRef listener);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// RunloopStream
///////////////////////////////////////////////////////////////////////////////////////////////////////////
RunloopStreamRef runloop_stream_new(RunloopRef runloop, int fd);
void runloop_stream_free(RunloopStreamRef stream);
void runloop_stream_init(RunloopStreamRef stream, RunloopRef runloop, int fd);
void runloop_stream_deinit(RunloopStreamRef stream);
void runloop_stream_register(RunloopStreamRef stream);
void runloop_stream_deregister(RunloopStreamRef stream);
void runloop_stream_arm_both(RunloopStreamRef stream,
                             PostableFunction read_postable_cb, void* read_arg,
                             PostableFunction write_postable_cb, void* write_arg);

void runloop_stream_arm_read(RunloopStreamRef stream, PostableFunction postable_callback, void* arg);
void runloop_stream_arm_write(RunloopStreamRef stream, PostableFunction postable_callback, void* arg);
void runloop_stream_disarm_read(RunloopStreamRef stream);
void runloop_stream_disarm_write(RunloopStreamRef stream);
void runloop_stream_verify(RunloopStreamRef stream);
RunloopRef runloop_stream_get_runloop(RunloopStreamRef stream);
int runloop_stream_get_fd(RunloopStreamRef stream);
void runloop_stream_checktag(RunloopStreamRef stream);

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
void runloop_user_event_free(RunloopUserEventRef uevent);
void runloop_user_event_register(RunloopUserEventRef uevent);
void runloop_user_event_arm(RunloopUserEventRef uevent, UserEventCallback cb, void* cb_arg);
void runloop_user_event_disarm(RunloopUserEventRef uevent);
#if defined(__linux__)
void runloop_user_event_fire(RunloopUserEventRef uevent);
#elif defined(__APPLE__)
void runloop_user_event_fire(RunloopUserEventRef uevent, void* data);
#endif
void runloop_user_event_deregister(RunloopUserEventRef uevent);
void runloop_user_event_verify(RunloopUserEventRef uevent);
RunloopRef runloop_user_event_get_runloop(RunloopUserEventRef uevent);

void runloop_user_event_init(RunloopUserEventRef uevent, RunloopRef runloop);
void runloop_user_event_change_watch(RunloopUserEventRef uevent, UserEventCallback cb, void* cb_arg, uint64_t watch_what);
void runloop_user_event_clear_one_event(RunloopUserEventRef uevent);
void runloop_user_event_clear_all_events(RunloopUserEventRef uevent);
int runloop_user_event_get_fd(RunloopUserEventRef uevent);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// User Event Queue
///////////////////////////////////////////////////////////////////////////////////////////////////////////
UserEventQueueRef user_event_queue_new(RunloopRef runloop, size_t capacity);
void user_event_queue_init(RunloopRef runloop, UserEventQueueRef uequeue, size_t capacity);
void user_event_queue_deinit(UserEventQueueRef uequeue);
void user_event_queue_free(UserEventQueueRef uequeue);
void user_event_queue_add(UserEventQueueRef uequeue, Functor item);
void user_event_queue_arm(UserEventQueueRef uequeue);
void user_event_queue_verify(UserEventQueueRef uequeue);
RunloopRef user_event_queue_get_runloop(UserEventQueueRef uequeue);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Base event
///////////////////////////////////////////////////////////////////////////////////////////////////////////
RunloopRef runloop_watcher_base_get_runloop(RunloopEventBaseRef watcher);
int        runloop_watcher_base_get_fd(RunloopEventBaseRef watcher);

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