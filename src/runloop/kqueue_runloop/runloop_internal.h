
#ifndef H_kqueue_runloop_internal_H
#define H_kqueue_runloop_internal_H
#include <runloop/runloop.h>
#include "rl_events_internal.h"
#include "kqueue_helpers.h"
void* runloop_event_allocate(RunloopRef rl, size_t size);
void runloop_event_free(RunloopRef rl, void* p);


#endif