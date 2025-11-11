#include <src/runloop/event_table.h>
#include "src/runloop/epoll_runloop/runloop_internal.h"
#include <assert.h>
#define RL_ALLO_EVENT_TABLE
void* rl_event_allocate(RunloopRef rl, size_t size)
{
    uint16_t objsize = object_pool_obj_size(rl->event_table_ref);
    assert(objsize >= size);
    void* p = event_table_get_entry(rl->event_table_ref);
    return p;
}

void rl_event_free(RunloopRef rl, void* p)
{
    event_table_release_entry(rl->event_table_ref, p);
}