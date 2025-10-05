#include <kqueue_runloop/event_allocator.h>
#include <kqueue_runloop/runloop.h>
#include <kqueue_runloop/rl_internal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

void freelist_init(FreeListRef fl, uint16_t max)
{
    fl->count = 0;
    fl->rdix = 0;
    fl->wrix = 0;
    fl->max = max;
    for(int i = 0; i < max - 1; i++) {
        freelist_add(fl, i);
    }
}
bool freelist_is_full(FreeListRef fl)
{
    return ((fl->wrix + 1) % fl->max) == fl->rdix;
}
bool freelist_is_empty(FreeListRef fl)
{
    return (fl->rdix == fl->wrix);
}
void freelist_add(FreeListRef fl, uint16_t element)
{
    assert(!freelist_is_full(fl));
    fl->buffer[fl->wrix] = element;
    fl->wrix = (fl->wrix + 1) % fl->max; 
}
uint16_t freelist_get(FreeListRef fl)
{
    assert(!freelist_is_empty(fl));
    uint16_t v = fl->buffer[fl->rdix];
    fl->rdix = (fl->rdix + 1) % fl->max;
    return v;
}
EventTable* event_allocator_new()
{
    void* m = malloc(sizeof(EventTable));
    assert(m != NULL);
    event_allocator_init(m);
    return m;
}
void event_allocator_init(EventTableRef ot)
{
    freelist_init(&(ot->free_list), runloop_MAX_EVENTS);
}
void* event_allocator_alloc(EventTableRef ot)
{
    uint16_t ix = freelist_get(&(ot->free_list));
    MemorySlab* mp = &(ot->memory[ix]);
    (mp->m).my_index = ix; 
    return mp;
}
void event_allocator_free(EventTableRef ot, void* p)
{
    MemorySlab* mp = (MemorySlab*)p;
    uint16_t ix = (mp->m).my_index;
    freelist_add(&(ot->free_list), ix);
}
bool event_allocator_has_outstanding_events(EventTableRef ot)
{
    return ! freelist_is_full(&(ot->free_list));
}
