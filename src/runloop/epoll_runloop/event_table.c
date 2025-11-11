#include "event_table.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

typedef union Mslab_u {
    RunloopTimer       timer;
    RunloopListener    listener;
    RunloopStream      stream;
    RunloopUserEvent   user_event;
    // RunloopQueueEvent   qevent;
    // RunloopInterthreadQueueEvent itqevent;
    // RunloopQueueWatcher qwatcher;
    // RunloopEvent        runloop_event;
} Mslab;

EventTableRef event_table_new()
{
    ObjectPool* op = object_pool_create(sizeof(Mslab), EVT_MAX);
    return (void*)op;
}
void event_table_free(void* op)
{
    object_pool_destroy((ObjectPoolRef)op);
}
void* event_table_get_entry(EventTableRef op)
{
    return object_pool_allocate(op);
}
void event_table_release_entry(EventTableRef op, void* p)
{
    object_pool_deallocate((ObjectPoolRef)op, p);
}
size_t event_table_number_in_use(EventTableRef op)
{
    return object_pool_number_in_use((ObjectPoolRef)op);
}
bool event_table_has_outstanding_events(EventTableRef op)
{
    object_pool_has_outstanding_objects((ObjectPoolRef)op);
}
