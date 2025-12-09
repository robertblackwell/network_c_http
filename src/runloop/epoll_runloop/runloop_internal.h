#ifndef H_epoll_runloop_internal_H
#define H_epoll_runloop_internal_H
#include <common/object_pool.h>
#include <runloop/runloop.h>
#include <runloop/rl_internal.h>
#include "rl_events_internal.h"

void* rl_event_allocate(RunloopRef rl, size_t size);
void rl_event_free(RunloopRef rl, void* p);

#endif