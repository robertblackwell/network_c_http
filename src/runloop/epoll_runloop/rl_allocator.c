#include <src/runloop/epoll_runloop/event_table.h>
#include "runloop_internal.h"
#include <assert.h>
#define RL_ALLO_EVENT_TABLE
void* rl_event_allocate(RunloopRef rl, size_t size)
{
#if defined(RL_ALLO_MALLOC)
    void* p = malloc(size);
    assert(p);
    return p;
    #elif defined(RL_ALLO_EVENT_TABLE)
    void* p = event_table_safe_get_entry(rl->event_table_ref, size, __FILE__, __LINE__);
    return p;
    #endif
}

void rl_event_free(RunloopRef rl, void* p)
{
#if defined(RL_ALLO_MALLOC)
    free(p);
    #elif defined(RL_ALLO_EVENT_TABLE)
    event_table_release_entry(rl->event_table_ref, p);
    #endif
}