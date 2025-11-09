
#ifndef H_MyType_allocator_H
#define H_MyType_allocator_H
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
typedef struct MyTypeTable_s MyTypeTable, *MyTypeTableRef;


MyTypeTableRef my_type_table_new();
void my_type_table_init(MyTypeTableRef et);
void* my_type_table_get_entry(MyTypeTableRef et);
void my_type_table_release_entry(MyTypeTableRef et, void* p);
bool my_type_table_has_outstanding_events(MyTypeTableRef et);
size_t my_type_table_number_in_use(MyTypeTableRef et);

#endif
