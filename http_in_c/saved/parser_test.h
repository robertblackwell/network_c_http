#ifndef c_c_http_parser_test_h
#define c_c_http_parser_test_h
#include <http_in_c/common/list.h>
#include <http_in_c/saved/rdsocket.h>
#include <http_in_c/http/message.h>
#include <http_in_c/saved/sync_reader.h>
#include <http_in_c/http/parser.h>

/**
 * A verify_function_t is a callable that examines the results of a parse test
 * to see that the results are as expected;
 */
typedef int(*verify_function_t)(ListRef msg_list)  ;

/** 
 * A parser test set consists of a descriptions, array of input lines or buffers,
 * and a verify_function that can verify the correctness of the outcome
 * from parsing those input buffers
*/
typedef struct parser_test_case_s
{
    char*               description;
    char**              lines;
    verify_function_t   verify_function;

    // the next field is a NULL terminated array of char*
} parser_test_case_t, *parser_test_case_r;

/**
 * WARNING - the args to this function must stay in existence for the life time of the
 * returned parser_test_case_r
 * @param description
 * @param lines
 * @param vf
 * @return
 */
parser_test_case_r parser_test_case_new(char* description, char** lines, verify_function_t vf);


typedef struct parse_result_s {
    MessageRef  message;
    int         rc;
} parse_result_t, *parse_result_r;

parse_result_r parse_result_new(MessageRef msg, int rc);
void parse_result_dispose(parse_result_r* this_ptr);


/**
 * This class runs an array of parser_test_case_t to make it easier to test the Parser
 * implementation on different sets of test data
 * 
 * Test data may consisting of multiple back to back messages,
 * and also correctly handles messages that require EOF to signal end-of-message
 */
typedef struct WrappedParserTest_s
{
    datasource_t*      m_data_source;
    verify_function_t  m_verify_func;
    ListRef            m_results;
    RdSocket            m_rdsock;
    SyncReaderRef              m_rdr;

    char                m_read_buffer[1000];
    char*               m_readbuffer_ptr;
    int                 m_buffer_length;
    int                 m_buffer_remaining;

} WrappedParserTest, *WrappedParserTestRef;
    
void WPT_init(WrappedParserTestRef this, datasource_t* data_source, verify_function_t verify_func);
//void WPT_destroy(WrappedParserTestRef this);

int WPT_run(WrappedParserTestRef this);

/** @} */
#endif