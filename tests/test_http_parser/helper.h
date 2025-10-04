#ifndef c_http_tests_test_parser_helper_h
#define c_http_tests_test_parser_helper_h
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <rbl/unittest.h>
#include <rbl/logger.h>
#include <src/common/list.h>
#include <src/http/kvpair.h>
#include <src/http/header_list.h>
#include <src/http_protocol/http_message.h>

int run_list (ListRef tests);

#define CHECK_HEADER(h, K, V) do {\
    KVPairRef hlref = HdrList_find(h, HEADER_HOST); \
    UT_NOT_EQUAL_PTR(hlref, NULL); \
    UT_EQUAL_CSTR(KVPair_label(hlref), HEADER_HOST); \
    UT_EQUAL_CSTR(KVPair_value(hlref), "ahost"); \
} while(0);

#define CHECK_BODY(M, S) do {\
    BufferChainRef body = http_message_get_body(M); \
    bool x##M = BufferChain_eq_cstr(body, S); \
    UT_EQUAL_INT(x##M, 1); \
} while(0);

#include "test_harness.h"

int test_requests();
int test_responses();
#endif