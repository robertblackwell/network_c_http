#ifndef c_c_eg_parser_test_h
#define c_c_eg_parser_test_h
#include <c_eg/list.h>
#include <c_eg/rdsocket.h>
#include <c_eg/message.h>
#include <c_eg/reader.h>
#include <c_eg/parser.h>
/**
 * A VerifyFunction is a callable that examines a MsgList to
 * check that is gives the expected result;
 */
typedef int(*VerifyFunctionType)(List* msg_list)  ;

/** 
 * A parser test set consists of a descriptions, array of input lines or buffers,
 * and a verify_function that can verify the correctness of the outcome
 * from parsing those input buffers
*/
typedef struct ParserTest_s 
{
    char*               description;
    char**              lines;
    VerifyFunctionType  verify_function;
    // the next field is a NULL terminated array of char*
} ParserTest, *ParserTestRef;

/**
 * WARNING - the args to this function must stay in existence for the life time of the
 * returned ParserTestRef
 * @param description
 * @param lines
 * @param vf
 * @return
 */
ParserTestRef ParserTest_new(char* description, char** lines, VerifyFunctionType vf);


typedef struct ReadResult_s {
    Message*  message;
    int         rc;
} ReadResult, *ReadResultRef;

ReadResultRef ReadResult_new(Message* msg, int rc);
void ReadResult_free(ReadResultRef* this_ptr);


/**
 * This class runs an array of ParserTest to make it easier to test the Parser 
 * implementation on different sets of test data
 * 
 * Test data may consisting of multiple back to back messages,
 * and also correctly handles messages that require EOF to signal end-of-message
 */
typedef struct WrappedParserTest_s
{
    Parser*           m_parser;
    DataSource*       m_data_source;
    VerifyFunctionType  m_verify_func;
    List*             m_results;
    RdSocket            m_rdsock;
    Rdr*              m_rdr;

    char                m_read_buffer[1000];
    char*               m_readbuffer_ptr;
    int                 m_buffer_length;
    int                 m_buffer_remaining;

} WrappedParserTest, *WrappedParserTestRef;
    
void WPT_init(WrappedParserTestRef this, Parser* parser, DataSource* data_source, VerifyFunctionType verify_func);
//void WPT_destroy(WrappedParserTestRef this);

int WPT_run(WrappedParserTestRef this);

#endif