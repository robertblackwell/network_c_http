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

int kqh_user_event_register(RunloopUserEventRef rlevent);
int kqh_user_event_trigger(RunloopUserEventRef rlevent, void* data);
int kqh_user_event_cancel(RunloopUserEventRef rlevent);
int kqh_user_event_pause (RunloopUserEventRef rlevent);

int kqh_user_event_queue_register(UserEventQueueRef rlevent);
int kqh_user_event_queue_trigger(UserEventQueueRef rlevent, void* data);
int kqh_user_event_queue_cancel(UserEventQueueRef rlevent);
int kqh_user_event_queue_pause (UserEventQueueRef rlevent);


int kqh_listener_register(RunloopListenerRef rlevent);
int kqh_listener_cancel(RunloopListenerRef rlevent);
int kqh_listener_pause (RunloopListenerRef rlevent);
#endif