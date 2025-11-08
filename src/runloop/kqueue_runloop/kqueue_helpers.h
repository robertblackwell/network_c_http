#ifndef H_kqueue_helpers_H
#define H_kqueue_helpers_H
#include "runloop_internal.h"
#include <sys/event.h>

int kqh_readerwriter_register(RunloopStreamRef rlevent);
int kqh_readerwriter_cancel(RunloopStreamRef rlevent);
int kqh_readerwriter_pause(RunloopStreamRef rlevent);

int kqh_reader_register(RunloopStreamRef rlevent);
int kqh_reader_cancel(RunloopStreamRef rlevent);
int kqh_reader_pause(RunloopStreamRef rlevent);

int kqh_writer_register(RunloopStreamRef rlevent);
int kqh_writer_cancel(RunloopStreamRef rlevent);
int kqh_writer_pause(RunloopStreamRef rlevent);

int kqh_signal_register(RunloopSignalRef rlevent);
int kqh_signal_cancel(RunloopSignalRef rlevent);
int kqh_signal_pause(RunloopSignalRef rlevent);

int kqh_timer_register(RunloopTimerRef rlevent, bool one_shot, uint64_t milli_secs);
int kqh_timer_cancel(RunloopTimerRef rlevent);
int kqh_timer_pause(RunloopTimerRef rlevent);

/**
 * Add an ident+filter of type EVFILT_USER to the runloop's kqueue
 * and set the filter to watch for trigger event with EV_DISPATCH set
 * so that the ident+filter must be re-armed after each event.
 *
 * In the case of kqueue this function can be called multiple times without problem.
 */
int kqh_user_event_arm(RunloopUserEventRef user_event);
/**
 * Trigger the user event (EVFILT_USER) and pass the given data value along with the event
 */
int kqh_user_event_trigger(RunloopUserEventRef user_event, void* data);
/**
 * Delete the ident+filter from the kqueue
 */
int kqh_user_event_cancel(RunloopUserEventRef user_event);
/**
 * Disable the ident+filter so that events are ignored by the kqueue
 */
int kqh_user_event_pause (RunloopUserEventRef user_event);


int kqh_listener_register(RunloopListenerRef rlevent);
int kqh_listener_cancel(RunloopListenerRef rlevent);
int kqh_listener_pause (RunloopListenerRef rlevent);
#endif