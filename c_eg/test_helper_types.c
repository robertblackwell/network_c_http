#define _GNU_SOURCE

#include <c_eg/test_helper_types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <c_eg/unittest.h>
#include <c_eg/utils.h>

ParserTestRef ParserTest_new(char* description, char** lines, VerifyFunctionType vf)
{
    ParserTestRef this = malloc(sizeof(ParserTest));
    this->description = description;
    this->lines = lines;
    this->verify_function = vf;
}


/**
 *
 * @param this DataSourceRef
 * @param blocks a pointer to an array of char* (an array of const cstring pointers)
 */
void DataSource_init(DataSourceRef this, char** blocks)
{
    this->m_block_count = 0;
    this->m_blocks = blocks;
}

/**
 * Returns the next block of utf-8 null terminated data, NULL when done
 * @return char*, NULL when done
 */
char* DataSource_next(DataSourceRef this)
{
    char* block = this->m_blocks[this->m_block_count];
    this->m_block_count++;
    return block;
}
/**
 * @return bool true when no more data
 */
bool DataSource_finished(DataSourceRef this)
{
    return (this->m_blocks[this->m_block_count] == NULL);
}

int DataSource_read(DataSourceRef this, void* buffer, int length)
{
    char* block = this->m_blocks[this->m_block_count];
    if (block == NULL) {
        return 0;
    }
    this->m_block_count++;
    int block_len = strlen(block);
    assert(block_len < length);
    memcpy((void*)buffer, block, block_len);
    return block_len;
}

static void msg_dealloc(void* p)
{
    void** pp = &p;
    Message_free((MessageRef)pp);
}
void WPT_init(WrappedParserTestRef this, ParserRef parser, DataSourceRef data_source, VerifyFunctionType verify_func)
{
    ASSERT_NOT_NULL(this);
    this->m_parser = parser;
    this-> m_data_source = data_source;
    this->m_verify_func = verify_func;
    this->m_messages = List_new(msg_dealloc);
}
WrappedParserTestRef WPT_new(WrappedParserTestRef this, ParserRef parser, DataSourceRef data_source, VerifyFunctionType verify_func)
{
    ASSERT_NOT_NULL(this);
    WrappedParserTestRef wptref = malloc(sizeof(WrappedParserTest));
    if(wptref == NULL)
        return NULL;
    WPT_init(wptref, parser, data_source, verify_func);
    return wptref;
}
void WPT_destroy(WrappedParserTestRef this)
{
    ASSERT_NOT_NULL(this);

}
void WPT_free(WrappedParserTestRef* this_ptr)
{
    ASSERT_NOT_NULL(*this_ptr);

}
// runs a test set and records pass/fail
int WPT_run(WrappedParserTestRef this)
{
    char buffer[1000];
    int buffer_len = 1000;
    MessageRef message_ptr = Message_new();
    Parser_begin(this->m_parser, message_ptr);
    int bytes_read;
    while(true) {
        bytes_read = DataSource_read(this->m_data_source, buffer, buffer_len);
        if (bytes_read == 0) {
            /// example of eof processing
            ParserReturnValue ret = Parser_end(this->m_parser);
            switch(ret.return_code) {
                case ParserRC_error:
                    assert(false);
                case ParserRC_end_of_data:
                case ParserRC_end_of_header:
                    break;
                case ParserRC_end_of_message:
                    List_add_back(this->m_messages, (void*)message_ptr);
                    message_ptr = Message_new();
                    Parser_begin(this->m_parser, message_ptr);
                break;
            }
            break;
        }
        char* buffer_ptr = buffer;
        int buffer_remaining_len = bytes_read;
        while (buffer_remaining_len > 0) {
            char* get_b = buffer_ptr;
            int get_sz = buffer_remaining_len;
            char* b = get_b;
            int len = get_sz;
            ParserReturnValue ret = Parser_consume(this->m_parser, (void*) b, len);
            ///
            /// keep track of data comsumed
            ///
            int consumed = len - ret.bytes_remaining;
            buffer_ptr = buffer_ptr + consumed;
            buffer_remaining_len = buffer_remaining_len - consumed;
            switch(ret.return_code) {
                case ParserRC_error:
                    assert(false);
                break;
                case ParserRC_end_of_data:
                break;
                case ParserRC_end_of_header:
                break;
                case ParserRC_end_of_message:
                    List_add_back(this->m_messages, (void*)message_ptr);
                    message_ptr = Message_new();
                    Parser_begin(this->m_parser, message_ptr);
                break;
            }
        }
    }
    return this->m_verify_func(this->m_messages);
}

