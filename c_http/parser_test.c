#define _GNU_SOURCE

#include <c_http/parser_test.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <c_http/alloc.h>

#include <c_http/unittest.h>
#include <c_http/utils.h>
#include <c_http/buffer/iobuffer.h>
#include <c_http/rdsocket.h>
#include <c_http/ll_reader.h>

ParserTestRef ParserTest_new(char* description, char** lines, VerifyFunctionType vf)
{
    ParserTestRef this = eg_alloc(sizeof(ParserTest));
    this->description = description;
    this->lines = lines;
    this->verify_function = vf;
}



ReadResultRef ReadResult_new(MessageRef msg, int rc)
{
    ReadResultRef rdref = eg_alloc(sizeof(ReadResult));
    rdref->message = msg;
    rdref->rc = rc;
}
void ReadResult_free(ReadResultRef* this_ptr)
{
    ReadResultRef this = *this_ptr;
    if(this->message != NULL)
        Message_free(&(this->message));
}

static void read_result_dealloc(void** p)
{
    void** pp = p;
    ReadResult_free((ReadResultRef*) p);
}


void WPT_init(WrappedParserTestRef this, ParserRef parser, DataSource* data_source, VerifyFunctionType verify_func)
{
    ASSERT_NOT_NULL(this);
    this->m_parser = parser;
    this-> m_data_source = data_source;
    this->m_verify_func = verify_func;
    this->m_results = List_new(read_result_dealloc);
    this->m_rdsock = DataSourceSocket(this->m_data_source);
    this->m_rdr = Reader_new(this->m_parser, this->m_rdsock);
}

void WPT_destroy(WrappedParserTestRef this)
{
    ASSERT_NOT_NULL(this);
}
#ifdef lslsl
int WPT_read_msg(WrappedParserTestRef this, IOBufferRef ctx, MessageRef* msgref_ptr )
{
    MessageRef message_ptr = Message_new();

    Parser_begin(this->m_parser, message_ptr);
    int bytes_read;
    for(;;) {
        if(IOBuffer_data_len(ctx) == 0 ) {
            IOBuffer_reset(ctx);
            message_ptr = Reader_read(this->m_rdr);
//            bytes_read = rdsocket.read_f(this->m_rdsocket.ctx, IOBuffer_space(ctx), IOBuffer_space_len(ctx));

            bytes_read = DataSource_read(this->m_data_source, IOBuffer_space(ctx), IOBuffer_space_len(ctx));
            // bytes_read == 0 means other end closed the socket,
            //      if have already started but not finished a message , send 0 bytes to parser to signal EOF
            //      if not started a new message did not get anything - no message available return NULL
            // bytes_read < 0 means io error which includes I closed the socket on the last iteration of the read message loop
            // in either case return NULL unless the other end is signalling end of message with a close (non HTTP spec compliant)

            if(bytes_read == 0) {
                if((bytes_read == 0) && this->m_parser->m_started && (!this->m_parser->m_message_done)) {
                    bytes_read = 0;
                } else {
                    Message_free(&(message_ptr));
                    *msgref_ptr = NULL;
                    return 0;
                }
            } else if (bytes_read < 0) {
                Message_free(&(message_ptr));
                *msgref_ptr = NULL;
                return -1;
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
                ///
                /// got a parse error - need some way to signal the caller so can send reply of bad message
                ///
                printf("Got error from parser %d\n", ret.return_code);
                ParserError pe = Parser_get_error(this->m_parser);
                printf("Error details %s %s \n", pe.m_name, pe.m_description);
//                assert(false);
                Message_free(&message_ptr);
                *msgref_ptr = NULL;
                return -2;
                break;
            case ParserRC_end_of_data:
                break;
            case ParserRC_end_of_header:
                break;
            case ParserRC_end_of_message:
                *msgref_ptr = message_ptr;
                return 0;
        }
    }
}
#endif
int WPT_run(WrappedParserTestRef this)
{
    MessageRef msgref;
    IOBuffer iobuf;
    IOBuffer_init(&iobuf, 256);
    int rc = 0;
    while(1) {
//        rc = WPT_read_msg(this, &iobuf, &msgref);
        rc = Reader_read(this->m_rdr, &msgref);
        ReadResultRef rr = ReadResult_new(msgref, rc);
        List_add_back(this->m_results, (void*)rr);
        if((rc != 0) || (msgref == NULL))
            break;
    }
    int r =this->m_verify_func(this->m_results);
    printf("Return from verify %d\n", r);
    return r;
}
