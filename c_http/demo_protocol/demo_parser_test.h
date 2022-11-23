#ifndef c_c_demo_parser_test_h
#define c_c_demo_parser_test_h
#include <c_http/common/list.h>
#include <c_http/common/http_parser/rdsocket.h>
#include <c_http/common/message.h>
#include <c_http//demo_protocol/demo_sync_reader.h>

/**
 * A VerifyFunction is a callable that examines a MsgList to
 * check that is gives the expected result;
 */
typedef int(*VerifyFunctionType)(ListRef msg_list)  ;

/** 
 * A parser test set consists of a descriptions, array of input lines or buffers,
 * and a verify_function that can verify the correctness of the outcome
 * from parsing those input buffers
*/
typedef struct DemoParserTest_s
{
    char*               description;
    char**              lines;
    VerifyFunctionType  verify_function;
    // the next field is a NULL terminated array of char*
} DemoParserTest, *DemoParserTestRef;

/**
 * WARNING - the args to this function must stay in existence for the life time of the
 * returned ParserTestRef
 * @param description
 * @param lines
 * @param vf
 * @return
 */
DemoParserTestRef DemoParserTest_new(char* description, char** lines, VerifyFunctionType vf);


typedef struct DemoReadResult_s {
    DemoMessageRef  message;
    int         rc;
} DemoReadResult, *DemoReadResultRef;

DemoReadResultRef DemoReadResult_new(DemoMessageRef msg, int rc);
void DemoReadResult_dispose(DemoReadResultRef* this_ptr);


/**
 * This class runs an array of ParserTest to make it easier to test the Parser 
 * implementation on different sets of test data
 * 
 * Test data may consisting of multiple back to back messages,
 * and also correctly handles messages that require EOF to signal end-of-message
 */
typedef struct DemoWrappedParserTest_s
{
    DataSource*         m_data_source;
    VerifyFunctionType  m_verify_func;
    ListRef             m_results;
    RdSocket            m_rdsock;
    DemoSyncReaderRef   m_rdr;

    char                m_read_buffer[1000];
    char*               m_readbuffer_ptr;
    int                 m_buffer_length;
    int                 m_buffer_remaining;

} DemoWrappedParserTest, *DemoWrappedParserTestRef;
    
void DemoWPT_init(DemoWrappedParserTestRef this, DataSource* data_source, VerifyFunctionType verify_func);
//void WPT_destroy(WrappedParserTestRef this);

int DemoWPT_run(DemoWrappedParserTestRef this);

/** @} */
#endif