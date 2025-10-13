#ifndef H_sample_object_table_h
#define sample_object_table_h
#include <kqueue_runloop/runloop.h>
#include <kqueue_runloop/rl_internal.h>
#include <kqueue_runloop/rl_events_internal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#include "tests/test_generic/object_table_generic.h"
typedef struct Sample_s {
    int one;
    int two;
}Sample, *SampleRef;

OBJECT_ALLOCATOR_HEADER("Sample", "sample", Sample, 100)

#endif