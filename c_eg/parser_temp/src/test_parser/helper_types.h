#ifndef c_c_eg_tests_parser_helpers_h
#define c_c_eg_tests_parser_helpers_h

/**
 * A VerifyFunction is a callable that examines a MsgList to
 * check that is gives the expected result;
 */
typedef void(*VerifyFunctionType)(MessageRef msg_list[])  ; 

/** 
 * A parser test set consists of a descriptions, array of input lines or buffers,
 * and a verify_function that can verify the correctness of the outcome
 * from parsing those input buffers
*/
typedef struct ParserTest_s 
{
    const char*               description;
    const char*               lines[];
    const VerifyFunctionType  verify_function;
} ParserTest, *ParserTestRef;


/**
 * Purpose of this class is to demo use of the parser in a situation
 * that simulates synchronously reading from some source of bytes
 * Generally a DataSource is created from the lines in a ParserTest
 */
typedef struct DataSource_s {
    // points as the current block being provided
    int   m_block_count;
    /** A array/list of data blocks null terminated*/
    char* m_blocks[];
} DataSource, *DataSourceRef

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
int DataSource_read_data(void* buffer, int length);
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
    MsgList             m_messages;

} WrappedParser, *WrappedParserRef
    
void WPT_init(WrappedParserTestRef this, ParserRef parser, DataSourceRef data_source, VerifyFunctionType verify_func);
WrappedParserRef WPT_new(WrappedParserTestRef this, ParserRef parser, DataSourceRef data_source, VerifyFunctionType verify_func);
void WPT_destroy(WrappedParserTestRef this);
void WPT_free(WrappedParserTestRef* this_ptr);

void WPT_run(WrappedParserTestRef this);

#endif