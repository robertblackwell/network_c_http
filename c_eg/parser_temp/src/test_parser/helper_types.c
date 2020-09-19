#ifndef c_c_eg_tests_parser_helpers_h
#define c_c_eg_tests_parser_helpers_h
#include "helper_types.h"

void DataSource_init(DataSourceRef this, char* blocks[])
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

int DataSource_read_data(void* buffer, int length)
{
    char* block = this->m_blocks[this->m_block_count];
    if (block == NULL) {
        return 0;
    }
    this->m_block_count++;
    int block_len = strlen(block);
    memcpy((void*)buffer, block, block_len);
    return block_len;
}

    
void WPT_init(WrappedParserTestRef this, ParserRef parser, DataSourceRef data_source, VerifyFunctionType verify_func)
{
    ASSERT_NOT_NULL(this);
    this->m_parser = parser;
    this-> m_data_source = data_source;
    this->m_verify_func = verify_func;
}
WrappedParserRef WPT_new(WrappedParserTestRef this, ParserRef parser, DataSourceRef data_source, VerifyFunctionType verify_func)
{
    ASSERT_NOT_NULL(this);
    WrappedParserRef wptref = malloc(sizeof(WrappedParser));
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
    ASSERT_NOT_NULL(*this);

}

void WPT_run(WrappedParserTestRef this)
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
            switch(ret->return_code) {
                case ParserRC_error:
                    REQUIRE(false);
                case ParserRC_end_of_data:
                case ParserRC_end_of_header:
                    break;
                case ParserRC_end_of_message:
                    m_messages.push_back((m_parser.current_message()));
                    message_ptr = Message_new();
                    Parser_begin(this->m_parser, message_ptr);
                break;
            }
            break;
        }
        char* buffer_ptr = buffer;
        int buffer_remaining_len = bytes_read;
        while (bytes_read > 0) {
            char* get_b = buffer_ptr;
            int get_sz = buffer_remaining_len;
            char* b = get_b;
            int len = get_sz;
            ParserReturnValue ret = Parser_consume(this->m_parser, (void*) b, len);
            ///
            /// keep track of data comsumed
            ///
            streambuffer.consume(len - ret.bytes_remaining);
            int consumed = len - ret.bytes_remaining;
            buffer_ptr = buffer_ptr + consumed;
            buffer_remaining_length = buffer_remaining_length - consumed;
            switch(ret.return_code) {
                case ParserRC_error:
                    REQUIRE(false);
                break;
                case ParserRC_end_of_data:
                break;
                case ParserRC_end_of_header:
                break;
                case ParserRC_end_of_message:
                    /// save the just parsed message
                    m_messages.push_back(message_sptr);
                    /// get ready for the next one
                    /// notice we keep processing the same buffer even with a 
                    /// new message
                    message_ptr = Message_new();
                    Parser_begin(this->m_parser, message_ptr);
                break;
            }
            auto z = streambuffer.data().data();
            char* z_p = (char*)z;
            auto z_sz = streambuffer.data().size();
            // std::cout << "z: " << z << " content: " << get_b <<  std::endl;
        }
    }
    m_verify_func(m_messages);
}


#endif