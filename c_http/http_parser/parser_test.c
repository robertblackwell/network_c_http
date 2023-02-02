#define _GNU_SOURCE

#include <c_http/http_parser/parser_test.h>
#include <stdio.h>
#include <assert.h>
#include <c_http/common/alloc.h>
#include <c_http/common/utils.h>
#include <c_http/common/iobuffer.h>
#include <c_http/http_parser/rdsocket.h>

parser_test_case_r parser_test_case_new(char* description, char** lines, verify_function_t vf)
{
    parser_test_case_r this = eg_alloc(sizeof(parser_test_case_t));
    this->description = description;
    this->lines = lines;
    this->verify_function = vf;
}

parse_result_r parse_result_new(MessageRef msg, int rc)
{
    parse_result_r rdref = eg_alloc(sizeof(parse_result_t));
    rdref->message = msg;
    rdref->rc = rc;
}
void parse_result_dispose(parse_result_r* this_ptr)
{
    parse_result_r this = *this_ptr;
    if(this->message != NULL)
        Message_dispose(&(this->message));
}

static void read_result_dealloc(void** p)
{
    void** pp = p;
    parse_result_dispose((parse_result_r *) p);
}


void WPT_init(WrappedParserTestRef this, datasource_t* data_source, verify_function_t verify_func)
{
    ASSERT_NOT_NULL(this);
    this-> m_data_source = data_source;
    this->m_verify_func = verify_func;
    this->m_results = List_new(read_result_dealloc);
    this->m_rdsock = DataSourceSocket(this->m_data_source);
}

void WPT_destroy(WrappedParserTestRef this)
{
    ASSERT_NOT_NULL(this);
}
void on_message_handler(ParserRef parser_ref, MessageRef msg_ref)
{
    WrappedParserTestRef ptest = parser_ref->handler_context;
    parse_result_r r = parse_result_new(msg_ref, 0);
    List_add_back(ptest->m_results, r);
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
                ParserError pe = Parser_get_error(this->m_parser);
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
int WPT_run(WrappedParserTestRef this)
{
    MessageRef msgref;
    IOBufferRef iobuf_ref = IOBuffer_new_with_capacity(256);
    datasource_t* ds_ptr = this->m_data_source;

    ParserRef pref = Parser_new(on_message_handler, (void*) this);
    while(1) {
        llhttp_errno_t rc;
//        char*data = datasource_next(ds_ptr);
//        int len = (data != NULL ) ? strlen(data): 0;
        char buffer[1000];
        char* b = &buffer;
        int length = 900;
        int status = datasource_read_some(ds_ptr, &buffer, length);
        if(status > 0) {
            rc = Parser_consume(pref, (void *) buffer, status);
            if(rc != HPE_OK) {
                printf("WPT_run status > 0 rc: %d\n", rc);
                List_add_back(this->m_results, parse_result_new(NULL, rc));
                break;
            }
        } else if(status == 0) {
            int x =
            rc = Parser_consume(pref, NULL, 0);
            if(rc != HPE_OK) {
                printf("WPT_run status == 0 rc: %d\n", rc);
                List_add_back(this->m_results, parse_result_new(NULL, rc));
            }
            break;
        } else {
            printf("WPT_run status < 0 io error rc: %d\n", HPE_USER);
            List_add_back(this->m_results, parse_result_new(NULL, HPE_USER));
            break;
        }
    }
    int r =this->m_verify_func(this->m_results);
    printf("Return from verify %d\n", r);
    IOBuffer_dispose(&iobuf_ref);
    return r;
}
