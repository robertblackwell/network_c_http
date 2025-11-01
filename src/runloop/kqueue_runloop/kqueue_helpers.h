#ifndef H_kqueue_helpers_H
#define H_kqueue_helpers_H
#include <kqueue_runloop/runloop_internal.h>
#include <sys/event.h>

int kqh_readerwriter_register(RunloopEventRef rlevent);
int kqh_readerwriter_cancel(RunloopEventRef rlevent);
int kqh_readerwriter_pause(RunloopEventRef rlevent);

int kqh_reader_register(RunloopEventRef rlevent);
int kqh_reader_cancel(RunloopEventRef rlevent);
int kqh_reader_pause(RunloopEventRef rlevent);

int kqh_writer_register(RunloopEventRef rlevent);
int kqh_writer_cancel(RunloopEventRef rlevent);
int kqh_writer_pause(RunloopEventRef rlevent);

int kqh_signal_register(RunloopEventRef rlevent);
int kqh_signal_cancel(RunloopEventRef rlevent);
int kqh_signal_pause(RunloopEventRef rlevent);

int kqh_timer_register(RunloopEventRef rlevent, bool one_shot, uint64_t milli_secs);
int kqh_timer_cancel(RunloopEventRef rlevent);
int kqh_timer_pause(RunloopEventRef rlevent);

int kqh_user_event_register(RunloopEventRef rlevent);
int kqh_user_event_trigger(RunloopEventRef rlevent, void* data);
int kqh_user_event_cancel(RunloopEventRef rlevent);
int kqh_user_event_pause (RunloopEventRef rlevent);

int kqh_listener_register(RunloopEventRef rlevent);
int kqh_listener_cancel(RunloopEventRef rlevent);
int kqh_listener_pause (RunloopEventRef rlevent);
#endif