#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <string.h>
//#include <c_http/common/http_parser/ll_parser_types.h>
#include <c_http/unittest.h>
#include <c_http/logger.h>
#include <c_http/common/kvpair.h>
#include <c_http/common/message.h>
#include <c_http/test_helpers/message_private.h>
#include <c_http/saved/sync_reader.h>
#include "test_harness.h"
int run_list (ListRef tests)
{
    int result = 0;
    // note all labels are upper case
    ListIterator iter = List_iterator (tests);
    for (;;) {
        if (iter == NULL) {
            break;
        }
        ListIterator next = List_itr_next (tests, iter);
        parser_test_t* current_test_ptr = (parser_test_t*) List_itr_unpack (tests, iter);
        result = result || parser_test_run(current_test_ptr);
        iter = next;
    }
    return result;
}
