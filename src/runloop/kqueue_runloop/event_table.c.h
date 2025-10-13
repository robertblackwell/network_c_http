
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
// typedef struct EventTable_s EventTable, *EventTableRef;

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

// initialize a free list
void freelist_init(FreeListRef fl);
// tests a free list to see if its full
bool freelist_is_full(FreeListRef fl);
// tests a free list to see if empty
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

struct EventTable_s {
    FreeList    free_list;
    MemorySlab  memory[EVT_MAX+1];
};

// create a new EventTable
EventTableRef event_table_new();
//Init an EventTable pass in the memory it will occupy
void event_table_init(EventTableRef et);
// get a free entry from an event table
void* event_table_get_entry(EventTableRef et);
// release an Event table entry back to the EventTable free list
void event_table_release_entry(EventTableRef et, void* p);
// returns true if there are any entries in use
bool event_table_has_outstanding_events(EventTableRef et);
// returns the number of EventTable entries in use
size_t event_table_number_in_use(EventTableRef et);

#endif