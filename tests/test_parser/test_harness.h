#ifndef c_http_test_parser_harness_h
#define c_http_test_parser_harness_h
#include <stdbool.h>
#include <c_http/common/list.h>
#include <c_http/common/message.h>
#include <c_http/saved/sync_reader.h>
#include <c_http/http_parser/ll_parser.h>

/* Test input - is a list of blocks of text and some context so that these blocks can be
 * "read" and "processed" one at a time*/
typedef struct test_input_s {
    const char*   description;
    int           block_count;
    const char**  blocks;
    int           errno;

} test_input_t;
void test_input_init(test_input_t* this, const char* description, const char* blocks[]);
const char* test_input_next(test_input_t* this);
bool test_input_finished(test_input_t* this);
int test_input_read_some(test_input_t* this, void* buffer, int length);

/**
 * test output is a list of a pair (a message pointer, a return code) which
 * represent the result of trying to parse a the data in a single `test_input`
 * instance. If the message pointer is not null the return code is HPE_OK
 */
typedef struct test_output_s {
    MessageRef  message;
    int         rc;
} test_output_t, *test_output_r;

test_output_r test_output_new(MessageRef msg, int rc);
void test_output_dispose(test_output_r* this_ptr);



/**
 * A verify_function_t is a callable that examines the results of a parse test
 * to see that the results are as expected;
 * The result of a parse test is a list of test_output_t*
 */
typedef int(*verify_function_t)(ListRef test_output_list)  ;

typedef struct parser_test_s
{
    test_input_t       test_input;
    ListRef            m_results; // a list of test_output_t*
    verify_function_t  m_verify_func;
//    char                m_read_buffer[1000];
//    char*               m_readbuffer_ptr;
//    int                 m_buffer_length;
//    int                 m_buffer_remaining;
} parser_test_t, *parser_test_r;
void parser_test_init(parser_test_t* this, const char* description, const char* blocks[], verify_function_t verify_func);
parser_test_t* parser_test_new(const char* description, const char* blocks[], verify_function_t vf);
int parser_test_run(parser_test_t* this);

#endif