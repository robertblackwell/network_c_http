#define _GNU_SOURCE

#include <c_eg/test_helper_types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <c_eg/alloc.h>

#include <c_eg/unittest.h>
#include <c_eg/utils.h>
#include <c_eg/buffer/iobuffer.h>

ParserTestRef ParserTest_new(char* description, char** lines, VerifyFunctionType vf)
{
    ParserTestRef this = eg_alloc(sizeof(ParserTest));
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

static void msg_dealloc(void** p)
{
    void** pp = p;
    Message_free((MessageRef*)pp);
}
void WPT_init(WrappedParserTestRef this, ParserRef parser, DataSourceRef data_source, VerifyFunctionType verify_func)
{
    ASSERT_NOT_NULL(this);
    this->m_parser = parser;
    this-> m_data_source = data_source;
    this->m_verify_func = verify_func;
    this->m_messages = List_new(msg_dealloc);
}

void WPT_destroy(WrappedParserTestRef this)
{
    ASSERT_NOT_NULL(this);

}

MessageRef WPT_read_msg(WrappedParserTestRef this, IOBufferRef ctx)
{
    MessageRef message_ptr = Message_new();
    Parser_begin(this->m_parser, message_ptr);
    int bytes_read;
    for(;;) {
        if(IOBuffer_data_len(ctx) == 0 ) {
            IOBuffer_reset(ctx);
            bytes_read = DataSource_read(this->m_data_source, IOBuffer_space(ctx), IOBuffer_space_len(ctx));
            if(bytes_read == 0) {
                if(this->m_parser->m_started && (!this->m_parser->m_message_done)) {
                    bytes_read = 0;
                } else {
                    return NULL;
                }
            }
            IOBuffer_commit(ctx, bytes_read);
        } else {
            bytes_read = ctx->buffer_remaining;
        }
        char* tmp = (char*)ctx->buffer_ptr;
        char* tmp2 = (char*)ctx->mem_p;
        ParserReturnValue ret = Parser_consume(this->m_parser, (void*) IOBuffer_data(ctx), IOBuffer_data_len(ctx));
        int consumed = bytes_read - ret.bytes_remaining;
        IOBuffer_consume(ctx, consumed);
        int tmp_remaining = ctx->buffer_remaining;
        switch(ret.return_code) {
            case ParserRC_error:
                assert(false);
                break;
            case ParserRC_end_of_data:
                break;
            case ParserRC_end_of_header:
                break;
            case ParserRC_end_of_message:
                return message_ptr;
        }
    }
}
int WPT_run(WrappedParserTestRef this)
{
    MessageRef msgref;
    IOBuffer context;
    IOBuffer_init(&context, 256);
    while((msgref = WPT_read_msg(this, &context)) != NULL) {
        List_add_back(this->m_messages, (void*)msgref);
    }
    int r =this->m_verify_func(this->m_messages);
    printf("Return from verify %d\n", r);
    return r;
}
