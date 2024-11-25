#ifndef C_HTTP_runloop_W_TIMER_H
#define C_HTTP_runloop_W_TIMER_H
#include "runloop.h"
/** \defgroup timer RunloopTimer
 * @{
*/
struct RunloopTimer_s;
typedef struct RunloopTimer_s RunloopTimer,  *RunloopTimerRef;      // Wait for a timer event

/**
 * A RunloopTimer is a spcial type of RunloopWatcherBase that make it easy to use a Runloop
 * to implement single shot and repeating timers.
*/
RunloopTimerRef runloop_timer_new(RunloopRef runloop_ref);
void runloop_timer_init(RunloopTimerRef this, RunloopRef runloop);
void runloop_timer_free(RunloopTimerRef athis);
void runloop_timer_register(RunloopTimerRef athis, PostableFunction cb, void* ctx, uint64_t interval_ms, bool repeating);
void runloop_timer_update(RunloopTimerRef athis, uint64_t interval_ms, bool repeating);
void runloop_timer_disarm(RunloopTimerRef athis);
void runloop_timer_rearm_old(RunloopTimerRef athis, PostableFunction cb, void* ctx, uint64_t interval_ms, bool repeating);
void runloop_timer_rearm(RunloopTimerRef athis);
void runloop_timer_deregister(RunloopTimerRef athis);
void WTimerFd_verify(RunloopTimerRef r);
RunloopRef runloop_timer_get_reactor(RunloopTimerRef athis);
int runloop_timer_get_fd(RunloopTimerRef this);
//#define runloop_timer_get_reactor(p) Watcher_get_reactor((RunloopWatcherBaseRef)p)
/**
 * Convenience interface for timers
 */
RunloopTimerRef runloop_timer_set(RunloopRef rl, PostableFunction cb, void* ctx, uint64_t interval_ms, bool repeating);
/**
 * After the call to runloop_timer_clear the timerref is invalid and muts not be ised
 */
void runloop_timer_clear(RunloopRef rl, RunloopTimerRef timerref);

/** @} */
#endif //C_HTTP_runloop_W_TIMER_H
