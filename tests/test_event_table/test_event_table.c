
#include <stdio.h>
#include <string.h>
#include <rbl/unittest.h>
#include <kqueue_runloop/runloop.h>
#include <kqueue_runloop/rl_internal.h>
#include <kqueue_runloop/rl_events_internal.h>
#include <kqueue_runloop/event_table.h>

int test_free_list()
{
    EventTable etvar;
    EventTableRef et = &etvar;
    FreeListRef fl = &(etvar.free_list);
    event_table_init(et);
    int x1 = event_table_number_in_use(et);
    UT_EQUAL_INT(x1, 0)
    bool b1 = freelist_is_full(fl);
    UT_TRUE(b1);
    size_t z1 = freelist_size(fl);
    UT_EQUAL_LONG((long)z1, (long)EVT_MAX)
    bool b2 = event_table_has_outstanding_events(et);
    UT_TRUE((!b2))
    int z2 = event_table_number_in_use(et);
    UT_EQUAL_INT(z2, 0)

    void* p1 = event_table_get_entry(et);
    {
        bool b1 = freelist_is_full(fl);
        UT_TRUE((!b1));
        size_t z1 = freelist_size(fl);
        UT_EQUAL_LONG((long)z1, (long)EVT_MAX-1)
        bool b2 = event_table_has_outstanding_events(et);
        UT_TRUE((b2))
        int z2 = event_table_number_in_use(et);
        UT_EQUAL_INT(z2, 1)
    }
    void* p2 = event_table_get_entry(et);
    void* p3 = event_table_get_entry(et);
    void* p4 = event_table_get_entry(et);
    void* p5 = event_table_get_entry(et);
    {
        int x2 = event_table_number_in_use(et);
        UT_EQUAL_INT(x2, 5)
        bool b2 = event_table_has_outstanding_events(et);
        UT_TRUE((b2))
    }
    event_table_release_entry(et, p1);
    event_table_release_entry(et, p2);
    event_table_release_entry(et, p3);
    event_table_release_entry(et, p4);
    event_table_release_entry(et, p5);
    {
        int x1 = event_table_number_in_use(et);
        UT_EQUAL_INT(x1, 0)
        bool b1 = freelist_is_full(fl);
        UT_TRUE(b1);
        size_t z1 = freelist_size(fl);
        UT_EQUAL_LONG((long)z1, (long)EVT_MAX)
        bool b2 = event_table_has_outstanding_events(et);
        UT_TRUE((!b2))
        int z2 = event_table_number_in_use(et);
        UT_EQUAL_INT(z2, 0)
    }
    event_table_init(et);
    for(int i = 0; i < EVT_MAX; i++) {

    }
    return 0;
}

int test_event_allocator()
{
    return 0;
}

int main()
{
    UT_ADD(test_free_list);
    int rc = UT_RUN();
    return rc;
}