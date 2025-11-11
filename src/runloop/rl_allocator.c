#include <common/object_pool.h>
#include <src/runloop/event_table.h>
#include "src/runloop/epoll_runloop/runloop_internal.h"
#include <assert.h>
#define RL_ALLO_EVENT_TABLE
void* rl_allocate_new(int obj_size, int obj_count)
{
    return object_pool_create(obj_size, obj_count);
}

void* rl_event_allocate(RunloopRef rl, size_t size)
{
    uint16_t objsize = object_pool_obj_size(rl->object_pool_ref);
    assert(objsize >= size);
    void* p = object_pool_allocate(rl->object_pool_ref);
    return p;
}

void rl_event_free(RunloopRef rl, void* p)
{
    event_table_release_entry(rl->object_pool_ref, p);
}