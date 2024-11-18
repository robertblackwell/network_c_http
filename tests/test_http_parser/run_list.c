
#include <assert.h>
#include <stdio.h>
#include <string.h>
//#include <http_in_c/common/http_parser/ll_parser_types.h>
#include <rbl/unittest.h>
#include <rbl/logger.h>
#include <http_in_c/http/kvpair.h>
#include <http_in_c/http/http_message.h>
#include <http_in_c/test_helpers/message_private.h>
//#include <http_in_c/saved/sync_reader.h>
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
