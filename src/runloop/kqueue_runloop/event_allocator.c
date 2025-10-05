#include <kqueue_runloop/event_allocator.h>
#include <kqueue_runloop/runloop.h>
#include <kqueue_runloop/rl_internal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

void freelist_init(FreeListRef fl)
{
    fl->count = 0;
    fl->rdix = 0;
    fl->wrix = 0;
    fl->max_entries = EVT_MAX;
    fl->modulo_max = EVT_MAX+1;
    for(int i = 0; i < fl->max_entries; i++) {
        freelist_add(fl, i);
    }
}
bool freelist_is_full(FreeListRef fl)
{
    return fl->count == (fl->max_entries);
}
bool freelist_is_empty(FreeListRef fl)
{
    return (fl->count == 0);
}
void freelist_add(FreeListRef fl, uint16_t element)
{
    assert(!freelist_is_full(fl));
    fl->count++;
    fl->buffer[fl->wrix] = element;
    fl->wrix = (fl->wrix + 1) % fl->modulo_max; 
}
uint16_t freelist_get(FreeListRef fl)
{
    assert(!freelist_is_empty(fl));
    uint16_t v = fl->buffer[fl->rdix];
    fl->count--;
    fl->rdix = (fl->rdix + 1) % fl->modulo_max;
    return v;
}
size_t freelist_size(FreeListRef fl)
{
    return fl->count;
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
    freelist_init(&(ot->free_list));
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
size_t event_allocator_number_in_use(EventTableRef et)
{

    FreeListRef fl = &(et->free_list);
    int fl_unused = freelist_size(fl);
    size_t fl_used = (fl->max_entries) - fl_unused;
    return fl_used;
}
bool event_allocator_has_outstanding_events(EventTableRef et)
{
    return ! freelist_is_full(&(et->free_list));
}
