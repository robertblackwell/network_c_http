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
typedef struct RunloopUserEvent_s RunloopUserEvent, *RunloopUserEventRef;      

typedef struct UserEventQueue_s UserEventQueue, * UserEventQueueRef;
typedef struct InterthreadQueue_s InterthreadQueue, *InterthreadQueueRef;
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
RunloopRef runloop_new(void);
void runloop_free(RunloopRef athis);
//void runloop_init(RunloopRef athis);
//void runloop_deinit(RunloopRef athis);
void runloop_close(RunloopRef athis);
//int runloop_register(RunloopRef athis, int fd, uint32_t interest, RunloopWatcherBaseRef wref);
//int runloop_deregister(RunloopRef athis, int fd);
//int runloop_reregister(RunloopRef athis, int fd, uint32_t interest, RunloopWatcherBaseRef wref);
int runloop_run(RunloopRef athis, int timeout_milli_secs);
void runloop_post(RunloopRef athis, PostableFunction cb, void* arg);
//void runloop_delete(RunloopRef athis, int fd);
void runloop_verify(RunloopRef r);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Timers
///////////////////////////////////////////////////////////////////////////////////////////////////////////
RunloopTimerRef runloop_timer_new(RunloopRef runloop_ref);
void runloop_timer_init(RunloopTimerRef timer, RunloopRef runloop);
void runloop_timer_free(RunloopTimerRef timer);
void runloop_timer_register(RunloopTimerRef timer, PostableFunction cb, void* ctx, uint64_t interval_ms, bool repeating);
void runloop_timer_update(RunloopTimerRef timer, PostableFunction cb, void* ctx, uint64_t interval_ms, bool repeating);
void runloop_timer_disarm(RunloopTimerRef timer);
void runloop_timer_rearm_old(RunloopTimerRef timer, PostableFunction cb, void* ctx, uint64_t interval_ms, bool repeating);
void runloop_timer_rearm(RunloopTimerRef timer);
void runloop_timer_deregister(RunloopTimerRef timer);
RunloopRef runloop_timer_get_runloop(RunloopTimerRef timer);

RunloopTimerRef runloop_timer_set(RunloopRef rl, PostableFunction cb, void* ctx, uint64_t interval_ms, bool repeating);
void runloop_timer_clear(RunloopRef rl, RunloopTimerRef timerref);
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

/**
 * epoll provides a facility to create a file descriptor that is not attached to any file/pipe/device
 * and to "fire" events on that file descriptor that can be waited for using the epoll call.
 * This facility provides a mechanism to create and wait on arbitary event sources.
 * 
 */
RunloopUserEventRef runloop_user_event_new(RunloopRef runloop);
void runloop_user_event_free(RunloopUserEventRef uevent);
void runloop_user_event_register(RunloopUserEventRef uevent);
void runloop_user_event_arm(RunloopUserEventRef uevent, PostableFunction cb, void* cb_arg);
void runloop_user_event_disarm(RunloopUserEventRef uevent);
void runloop_user_event_fire(RunloopUserEventRef uevent);
void runloop_user_event_deregister(RunloopUserEventRef uevent);
void runloop_user_event_verify(RunloopUserEventRef uevent);
RunloopRef runloop_user_event_get_runloop(RunloopUserEventRef uevent);

void runloop_user_event_init(RunloopUserEventRef uevent, RunloopRef runloop);
void runloop_user_event_change_watch(RunloopUserEventRef uevent, PostableFunction postable, void* arg, uint64_t watch_what);
void runloop_user_event_clear_one_event(RunloopUserEventRef uevent);
void runloop_user_event_clear_all_events(RunloopUserEventRef uevent);
int runloop_user_event_get_fd(RunloopUserEventRef uevent);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// User Event Queue
///////////////////////////////////////////////////////////////////////////////////////////////////////////
UserEventQueueRef user_event_queue_new(RunloopRef runloop);
void user_event_queue_init(RunloopRef runloop, UserEventQueueRef uequeue);
void user_event_queue_deinit(UserEventQueueRef uequeue);
void user_event_queue_free(UserEventQueueRef uequeue);
void user_event_queue_add(UserEventQueueRef uequeue, Functor item);
void user_event_queue_arm(UserEventQueueRef uequeue);
void user_event_queue_verify(UserEventQueueRef uequeue);
RunloopRef runloop_user_event_queue_get_runloop(UserEventQueueRef uequeue);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Base event
///////////////////////////////////////////////////////////////////////////////////////////////////////////
RunloopRef runloop_watcher_base_get_runloop(RunloopWatcherBaseRef athis);
int        runloop_watcher_base_get_fd(RunloopWatcherBaseRef athis);

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