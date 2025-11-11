#ifndef C_HTTP_runloop_allocator_H
#define C_HTTP_runloop_allocator_H
#include <runloop/runloop.h>
#include <runloop/functor.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <common/list.h>
typedef void* EventTableRef;

void* rl_allocate_new(int obj_size, int obj_count);
void* rl_event_allocate(RunloopRef rl, size_t size_required);
void  rl_event_free(RunloopRef rl, void* p);
#endif