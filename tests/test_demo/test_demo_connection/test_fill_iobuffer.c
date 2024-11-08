
#define RBL_LOG_ENABLE
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
             /* See feature_test_macros(7) */
#include <string.h>
#include <rbl/unittest.h>
#include <rbl/macros.h>
#include "fill_iobuffer.h"
#include <rbl/logger.h>

int test_fill_buffer()
{
    char* line = "thisisalineoftexttotest";
    IOBufferRef iob = fill_iobuffer(line, 1000, 300);
    UT_TRUE((IOBuffer_data_len(iob) > 300))
    return 0;
}

int main()
{
    UT_ADD(test_fill_buffer);
    int rc = UT_RUN();
    return rc;
}
