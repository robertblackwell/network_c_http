
#include <kqueue_runloop/event_table.c.h>
#include <kqueue_runloop/runloop.h>
#include <kqueue_runloop/rl_internal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
typedef struct FreeList_s FreeList, *FreeListRef;
typedef struct MemorySlab_s MemorySlab, *MemorySlabRef;
#define SLOTS_MAX 100
struct FreeList_s {
    uint16_t    count;
    uint16_t    max_entries;
    uint16_t    modulo_max;
    uint16_t    rdix;
    uint16_t    wrix;
    uint16_t    buffer[SLOTS_MAX+1];
};
struct MemorySlab_s {
    struct {
        MyType     mem;
        uint16_t    my_index; 
    } m;
};
struct MyTypeTable_s {
    FreeList    free_list;
    MemorySlab  memory[SLOTS_MAX+1];
};
static void freelist_init(FreeListRef fl)
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
static bool freelist_is_full(FreeListRef fl)
{
    return fl->count == (fl->max_entries);
}
static bool freelist_is_empty(FreeListRef fl)
{
    return (fl->count == 0);
}
static void freelist_add(FreeListRef fl, uint16_t element)
{
    assert(!freelist_is_full(fl));
    fl->count++;
    fl->buffer[fl->wrix] = element;
    fl->wrix = (fl->wrix + 1) % fl->modulo_max; 
}
static uint16_t freelist_get(FreeListRef fl)
{
    assert(!freelist_is_empty(fl));
    uint16_t v = fl->buffer[fl->rdix];
    fl->count--;
    fl->rdix = (fl->rdix + 1) % fl->modulo_max;
    return v;
}
static size_t freelist_size(FreeListRef fl)
{
    return fl->count;
}
MyTypeTable* my_type_table_new()
{
    void* m = malloc(sizeof(EventTable));
    assert(m != NULL);
    event_table_init(m);
    return m;
}
void my_type_table_init(MyTypeTableRef ot)
{
    freelist_init(&(ot->free_list));
}
void* my_type_table_get_entry(MyTypeTableRef ot)
{
    uint16_t ix = freelist_get(&(ot->free_list));
    MemorySlab* mp = &(ot->memory[ix]);
    (mp->m).my_index = ix; 
    return mp;
}
void my_type_table_release_entry(MyTypeTableRef ot, void* p)
{
    MemorySlab* mp = (MemorySlab*)p;
    uint16_t ix = (mp->m).my_index;
    freelist_add(&(ot->free_list), ix);
}
size_t my_type_table_number_in_use(MyTypeTableRef et)
{
    FreeListRef fl = &(et->free_list);
    int fl_unused = freelist_size(fl);
    size_t fl_used = (fl->max_entries) - fl_unused;
    return fl_used;
}
bool my_type_table_has_outstanding_events(MyTypeTableRef et)
{
    return ! freelist_is_full(&(et->free_list));
}
