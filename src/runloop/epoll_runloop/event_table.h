
#ifndef H_runloop_epoll_event_allocator_H
#define H_runloop_epoll_event_allocator_H
#include "runloop_internal.h"
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <common/object_pool.h>
#define EVT_MAX RL_MAX_EVENTS

EventTableRef event_table_new();
void event_table_init(EventTableRef et);
void* event_table_get_entry(EventTableRef et);
void* event_table_safe_get_entry(EventTableRef et, size_t obj_size, const char* file, int line);
void event_table_release_entry(EventTableRef et, void* p);
bool event_table_has_outstanding_events(EventTableRef et);
size_t event_table_number_in_use(EventTableRef et);
void event_table_free(EventTableRef et);

#endif