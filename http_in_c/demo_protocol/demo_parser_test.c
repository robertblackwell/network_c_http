

#include <http_in_c/demo_protocol/demo_parser_test.h>
#include <stdio.h>
#include <assert.h>
#include <http_in_c/common/alloc.h>
#include <http_in_c/common/utils.h>
#include <http_in_c/common/iobuffer.h>
#include <http_in_c/demo_protocol/demo_sync_reader_private.h>

DemoParserTestRef DemoParserTest_new(char* description, char** lines, verify_function_t vf)
{
    DemoParserTestRef this = eg_alloc(sizeof(DemoParserTest));
    this->description = description;
    this->lines = lines;
    this->verify_function = vf;
}



DemoReadResultRef DemoReadResult_new(DemoMessageRef msg, int rc)
{
    DemoReadResultRef rdref = eg_alloc(sizeof(DemoReadResult));
    rdref->message = msg;
    rdref->rc = rc;
}
void DemoReadResult_dispose(DemoReadResultRef* this_ptr)
{
    DemoReadResultRef this = *this_ptr;
    if(this->message != NULL)
        demo_message_dispose(&(this->message));
}

static void read_result_dealloc(void** p)
{
    void** pp = p;
    DemoReadResult_dispose((DemoReadResultRef*) p);
}


void DemoWPT_init(
        DemoWrappedParserTestRef this,
        datasource_t* data_source,
        verify_function_t verify_func)
{
    ASSERT_NOT_NULL(this);
    this-> m_data_source = data_source;
    this->m_verify_func = verify_func;
    this->m_results = List_new(read_result_dealloc);
    this->m_rdsock = DataSourceSocket(this->m_data_source);
    this->m_rdr = demosync_reader_private_new(this->m_rdsock);
}

void WPT_destroy(DemoWrappedParserTestRef this)
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
            message_ptr = SyncReader_read(this->m_rdr);
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
                    Message_dispose(&(message_ptr));
                    *msgref_ptr = NULL;
                    return 0;
                }
            } else if (bytes_read < 0) {
                Message_dispose(&(message_ptr));
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
                http_parser_error_t pe = Parser_get_error(this->m_parser);
                printf("Error details %s %s \n", pe.m_name, pe.m_description);
//                assert(false);
                Message_dispose(&message_ptr);
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
int DemoWPT_run(DemoWrappedParserTestRef this)
{
    DemoMessageRef msgref;
//    IOBufferRef iobuf_ref = IOBuffer_new_with_capacity(256);
    int rc = 0;
    while(1) {
        rc = demosync_reader_read(this->m_rdr, &msgref);
        if(msgref == NULL)
            // TODO - the reader needs a more over return of "read nothing"
            break;
        DemoReadResultRef rr = DemoReadResult_new(msgref, rc);
        List_add_back(this->m_results, (void*)rr);
        if((rc != 0) || (msgref == NULL))
            break;
    }
    int r =this->m_verify_func(this->m_results);
    printf("Return from verify %d\n", r);
//    IOBuffer_dispose(&iobuf_ref);
    return r;
}
