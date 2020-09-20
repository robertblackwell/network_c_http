#ifndef c_c_eg_tests_parser_helpers_h
#define c_c_eg_tests_parser_helpers_h
#include <c_eg/list.h>
#include <c_eg/message.h>
#include <c_eg/parser.h>
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

/**
 * Purpose of this class is to demo use of the parser in a situation
 * that simulates synchronously reading from some source of bytes
 * Generally a DataSource is created from the lines in a ParserTest
 */
typedef struct DataSource_s {
    // points as the current block being provided
    int   m_block_count;
    /** A array/list of data blocks null terminated*/
    char** m_blocks;
} DataSource, *DataSourceRef;

void DataSource_init(DataSourceRef this, char* blocks[]);

/**
 * Returns the next block of utf-8 null terminated data, NULL when done
 * @return char*, NULL when done
 */
char* DataSource_next(DataSourceRef this);

/**
 * @return bool true when no more data
 */
bool DataSource_finished(DataSourceRef this);

/*
* 'Reads' up to length data into buffer and returns the actually number 'read'
*/
int DataSource_read(DataSourceRef this, void* buffer, int length);
/**
 * This class runs an array of ParserTest to make it easier to test the Parser 
 * implementation on different sets of test data
 * 
 * Test data may consisting of multiple back to back messages,
 * and also correctly handles messages that require EOF to signal end-of-message
 */
typedef struct WrappedParserTest_s
{
    // using MsgList = std::vector<Marvin::MessageBase*>;
    // using VerifyFunctionType = std::function<void(MsgList msg_list)>;
    
    ParserRef           m_parser;
    DataSourceRef       m_data_source;
    VerifyFunctionType  m_verify_func;
    ListRef             m_messages;

} WrappedParserTest, *WrappedParserTestRef;
    
void WPT_init(WrappedParserTestRef this, ParserRef parser, DataSourceRef data_source, VerifyFunctionType verify_func);
WrappedParserTestRef WPT_new(WrappedParserTestRef this, ParserRef parser, DataSourceRef data_source, VerifyFunctionType verify_func);
void WPT_destroy(WrappedParserTestRef this);
void WPT_free(WrappedParserTestRef* this_ptr);

int WPT_run(WrappedParserTestRef this);
int WPT_run2(WrappedParserTestRef this);

#endif