
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
    // how many are on the list
    uint16_t    count;
    // the max number that can be put in the list
    uint16_t    max_entries;
    // the number to use for modulo arithmetic
    uint16_t    modulo_max;
    uint16_t    rdix;
    uint16_t    wrix;
    uint16_t    buffer[EVT_MAX+1];
};

void freelist_init(FreeListRef fl);
bool freelist_is_full(FreeListRef fl);
bool freelist_is_empty(FreeListRef fl);
// add an entry to the back of the list
void freelist_add(FreeListRef fl, uint16_t element);
// get and remove an entry from the front of the list
uint16_t freelist_get(FreeListRef fl);
// the number of entries on the list
size_t freelist_size(FreeListRef fl);


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
    MemorySlab  memory[EVT_MAX+1];
};

EventTableRef event_allocator_new();
void event_allocator_init(EventTableRef et);
void* event_allocator_alloc(EventTableRef et);
void event_allocator_free(EventTableRef et, void* p);
size_t event_allocator_number_outstanding(EventTable et);
bool event_allocator_has_outstanding_events(EventTableRef et);
size_t event_allocator_number_in_use(EventTableRef et);

#endif