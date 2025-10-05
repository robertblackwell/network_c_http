
#ifndef H_event_allocator_H
#define H_event_allocator_H
#include <kqueue_runloop/runloop.h>
#include <kqueue_runloop/rl_internal.h>
#include <kqueue_runloop/rl_events_internal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#define EVT_MAX runloop_MAX_EVENTS

typedef struct FreeList_s FreeList, *FreeListRef;
typedef struct MemorySlab_s MemorySlab, *MemorySlabRef;
typedef struct EventTable_s EventTable, *EventTableRef;

struct FreeList_s {
    uint16_t    count;
    uint16_t    max;
    uint16_t    rdix;
    uint16_t    wrix;
    uint16_t    buffer[EVT_MAX];
};

void freelist_init(FreeListRef fl, uint16_t max);
bool freelist_is_full(FreeListRef fl);
bool freelist_is_empty(FreeListRef fl);
void freelist_add(FreeListRef fl, uint16_t element);
uint16_t freelist_get(FreeListRef fl);


struct MemorySlab_s {
    struct {
        union {
            RunloopEventRef     timer;
            RunloopEventRef     listener;
            RunloopStream       stream;
            RunloopEvent        user_event;
            RunloopEventRef     signal;

            RunloopQueueEvent   qevent;
            RunloopInterthreadQueueEvent itqevent;
            RunloopQueueWatcher qwatcher;
            RunloopEvent        runloop_event;
        };
        uint16_t    my_index; 
    } m;
};

typedef struct EventTable_s {
    FreeList    free_list;
    MemorySlab  memory[EVT_MAX];
};

EventTableRef event_allocator_new();
void event_allocator_init(EventTableRef ot);
void* event_allocator_alloc(EventTableRef ot);
void event_allocator_free(EventTableRef ot, void* p);
bool event_allocator_has_outstanding_events(EventTableRef ot);
#endif