#ifndef C_HTTP_kq_runloop_TIMER_H
#define C_HTTP_kq_runloop_TIMER_H
#include <kqueue_runloop/runloop.h>
#include <kqueue_runloop/rl_internal.h>
#include <kqueue_runloop/rl_events_internal.h>

/**
 * @brief 
 * This function creates a pointer to a timer subtype of the tagged union RunloopEvent
 * @param runloop_ref 
 * @return RunloopEventRef a timer subtype 
 */
RunloopEventRef runloop_timer_new(RunloopRef runloop_ref);
/**
 * These functions all take a RunloopEventRef, which is a tagged union. 
 * All of them check that the begin and end tags as well as the union tag type
*/
void runloop_timer_init(RunloopEventRef lrevent, RunloopRef runloop);
void runloop_timer_free(RunloopEventRef lrevent);
void runloop_timer_register(RunloopEventRef lrevent, PostableFunction cb, void* ctx, uint64_t interval_ms, bool repeating);
void runloop_timer_update(RunloopEventRef lrevent, uint64_t interval_ms, bool repeating);
void runloop_timer_disarm(RunloopEventRef lrevent);
void runloop_timer_rearm_old(RunloopEventRef lrevent, PostableFunction cb, void* ctx, uint64_t interval_ms, bool repeating);
void runloop_timer_rearm(RunloopEventRef lrevent);
void runloop_timer_deregister(RunloopEventRef lrevent);
RunloopRef runloop_timer_get_runloop(RunloopEventRef lrevent);
/**
 * Convenience interface for timers
 */
RunloopEventRef runloop_timer_set(RunloopRef rl, PostableFunction cb, void* ctx, uint64_t interval_ms, bool repeating);
/**
 * After the call to runloop_timer_clear the timerref is invalid and muts not be ised
 */
void runloop_timer_clear(RunloopRef rl, RunloopEventRef lrevent);
void runloop_timer_checktag(RunloopEventRef lrevent);
#endif //C_HTTP_kq_runloop_TIMER_H
