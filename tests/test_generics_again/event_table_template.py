import sys
def h_template(type_name, func_prefix, nbr_slots):
    template = """
#ifndef H_@type_name@_allocator_H
#define H_@type_name@_allocator_H
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
typedef struct @type_name@Table_s @type_name@Table, *@type_name@TableRef;


@type_name@TableRef @func_prefix@_table_new();
void @func_prefix@_table_init(@type_name@TableRef et);
void* @func_prefix@_table_get_entry(@type_name@TableRef et);
void @func_prefix@_table_release_entry(@type_name@TableRef et, void* p);
bool @func_prefix@_table_has_outstanding_events(@type_name@TableRef et);
size_t @func_prefix@_table_number_in_use(@type_name@TableRef et);

#endif
"""
    s = template.replace("@type_name@", type_name).replace("@func_prefix@", func_prefix).replace("@nbr_slots@", nbr_slots)
    return s


def c_template(type_name, func_prefix, nbr_slots):
    template = """
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
#define SLOTS_MAX @nbr_slots@
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
        @type_name@     mem;
        uint16_t    my_index; 
    } m;
};
struct @type_name@Table_s {
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
@type_name@Table* @func_prefix@_table_new()
{
    void* m = malloc(sizeof(EventTable));
    assert(m != NULL);
    event_table_init(m);
    return m;
}
void @func_prefix@_table_init(@type_name@TableRef ot)
{
    freelist_init(&(ot->free_list));
}
void* @func_prefix@_table_get_entry(@type_name@TableRef ot)
{
    uint16_t ix = freelist_get(&(ot->free_list));
    MemorySlab* mp = &(ot->memory[ix]);
    (mp->m).my_index = ix; 
    return mp;
}
void @func_prefix@_table_release_entry(@type_name@TableRef ot, void* p)
{
    MemorySlab* mp = (MemorySlab*)p;
    uint16_t ix = (mp->m).my_index;
    freelist_add(&(ot->free_list), ix);
}
size_t @func_prefix@_table_number_in_use(@type_name@TableRef et)
{
    FreeListRef fl = &(et->free_list);
    int fl_unused = freelist_size(fl);
    size_t fl_used = (fl->max_entries) - fl_unused;
    return fl_used;
}
bool @func_prefix@_table_has_outstanding_events(@type_name@TableRef et)
{
    return ! freelist_is_full(&(et->free_list));
}
"""
    s = template.replace("@type_name@",type_name).replace("@func_prefix@", func_prefix).replace("@nbr_slots@", nbr_slots)
    return s
def main():
    type_name = sys.argv[1]
    func_prefix = sys.argv[2]
    nbr_slots = sys.argv[3]

    print(f"Creating module {type_name}Table ith func_prefix: {func_prefix} and nbr_slots: {nbr_slots}")

    h_output = h_template("MyType", "my_type", "100")
    c_output = c_template("MyType", "my_type", "100")

    with open("my_type_table.h", "w") as h:
        h.write(h_output)
    with open("my_type_table.c", "w") as c:
        c.write(c_output)

main()