#include <c_eg/alloc.h>
#include <c_eg/parser.h>
#include <c_eg/utils.h>

/// forward declares 
int chunk_header_cb(http_parser* parser);
int chunk_complete_cb(http_parser* parser);
int message_begin_cb(http_parser* parser);
int url_data_cb(http_parser* parser, const char* at, size_t length);
int status_data_cb(http_parser* parser, const char* at, size_t length);
int header_field_data_cb(http_parser* parser, const char* at, size_t length);
int header_value_data_cb(http_parser* parser, const char* at, size_t length);
int headers_complete_cb(http_parser* parser);//, const char* aptr, size_t remainder);
int body_data_cb(http_parser* parser, const char* at, size_t length);
int message_complete_cb(http_parser* parser);

void Parser_initialize(ParserRef this);

ParserRef Parser_new()
{
    ParserRef this = eg_alloc(sizeof(Parser));
    if(this == NULL)
        return NULL;
    this->m_message_done = false;
    this->m_header_done = false;
    this->m_http_parser_ptr = NULL;
    this->m_http_parser_settings_ptr = NULL;
    this->m_header_state = kHEADER_STATE_NOTHING;
    this->m_http_parser_ptr = NULL;
    this->m_http_parser_settings_ptr = NULL;
    return this;
}

void Parser_free(ParserRef* this_p)
{
    ASSERT_NOT_NULL(*this_p);
    ParserRef this= *this_p;
    if (this->m_http_parser_ptr != NULL) {
        free(this->m_http_parser_ptr);
        this->m_http_parser_ptr = NULL;
    }
    *this_p = NULL;

}
MessageRef Parser_current_message(ParserRef this)
{
    return this->m_current_message_ptr;
}
int Parser_appendBytes(ParserRef this, void *buffer, unsigned length)
{
    size_t nparsed = http_parser_execute(this->m_http_parser_ptr, this->m_http_parser_settings_ptr, (char*)buffer, (int)length);
    return (int)nparsed;
}
void Parser_begin(ParserRef this, MessageRef message_ptr)
{
    Parser_initialize(this);
    this->m_current_message_ptr = message_ptr;
}


ParserReturnValue Parser_consume(ParserRef this, const void* buf, int length)
{
    bool only_header = false;
    this->m_started = true;
    ParserReturnValue rv = {.return_code = ParserRC_end_of_data, .bytes_remaining = length};
    char* b = (char*) buf;
    size_t total_parsed = 0;
    while (total_parsed < length) {
        char* b_start_ptr = &(b[total_parsed]);
        int nparsed = Parser_appendBytes(this, (void*) b_start_ptr, length - total_parsed);
        total_parsed = total_parsed + nparsed;
        // std::cout << "nparsed: " << nparsed  << "len: " << length - total_parsed << " content: " << buf <<  std::endl;
        rv.bytes_remaining = length - nparsed;
        if (Parser_is_error(this)) {
            rv.return_code = ParserRC_error;
            ParserError x = Parser_get_error(this);
            return rv;
        } else if (this->m_message_done) {
            rv.return_code = ParserRC_end_of_message;
            return rv;
        } else if (this->m_header_done) {
            rv.return_code = ParserRC_end_of_header;
            if (only_header) {
                return rv;
            }
        } else if (nparsed == length) {

        }
    }
    return rv;
}
ParserReturnValue Parser_end(ParserRef this)
{
    ParserReturnValue rv = {.return_code = ParserRC_end_of_data, .bytes_remaining = 0};
    char* buffer = NULL;
    size_t nparsed;
    int someLength = 0;
    if( ! this->m_message_done ) {
        if (this->m_started) {
            nparsed = http_parser_execute(this->m_http_parser_ptr, this->m_http_parser_settings_ptr, buffer, someLength);
        }
        if (Parser_is_error(this)) {
            rv.return_code = ParserRC_error;
            ParserError x = Parser_get_error(this);
            return rv;
        } else if (this->m_message_done) {
            rv.return_code = ParserRC_end_of_message;
            return rv;
        } else if (this->m_header_done) {
            rv.return_code = ParserRC_end_of_header;
        } else {
            printf("should not be here\n");
            return rv;
        }
        return rv;
    }
    return rv;
}
void Parser_append_eof(ParserRef this)
{
    char* buffer = NULL;
    size_t nparsed;
    int someLength = 0;
    if( ! this->m_message_done )
    {
        nparsed = http_parser_execute(this->m_http_parser_ptr, this->m_http_parser_settings_ptr, buffer, someLength);
    }
    printf("back from parser nparsed: %ld\n ", nparsed);
}
enum http_errno Parser_get_errno(ParserRef this)
{
    return (enum http_errno) this->m_http_parser_ptr->http_errno;
}
ParserError Parser_get_error(ParserRef this)
{
    enum http_errno x = (enum http_errno)this->m_http_parser_ptr->http_errno;
    char* n = (char*)http_errno_name(x);
    char* d = (char*)http_errno_description(x);
    ParserError erst;
    erst.m_err_number = x;
    erst.m_name = n;
    erst.m_description = d;
    if (this->m_http_parser_ptr->http_errno != 0)
        printf(" errno: %d name: %s description: %s\n", this->m_http_parser_ptr->http_errno, n, d);
    return erst;

}
bool Parser_is_error(ParserRef this)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wall"
    enum http_errno x = (enum http_errno)this->m_http_parser_ptr->http_errno;
    char* n = (char*)http_errno_name(x);
    char* d = (char*)http_errno_description(x);
#pragma clang diagnostic pops
    if (this->m_http_parser_ptr->http_errno != 0)
        printf(" errno: %d name: %s description %s\n", this->m_http_parser_ptr->http_errno, n, d);
    // FTROG_DEBUG(" errno: %d name: %s, description: %s", this->parser->http_errno, n,d);
    return (this->m_http_parser_ptr->http_errno != 0) && (this->m_http_parser_ptr->http_errno != HPE_PAUSED);
};


void Parser_initialize(ParserRef this)
{
    this->m_header_state = kHEADER_STATE_NOTHING;
    this->m_started = false;
    this->m_message_done = false;
    this->m_header_done = false;
    this->m_current_message_ptr = NULL;
    if (this->m_http_parser_ptr != NULL) {
        free(this->m_http_parser_ptr);
    }
    this->m_http_parser_ptr = (http_parser*)eg_alloc(sizeof(http_parser));
    http_parser_init( this->m_http_parser_ptr, HTTP_BOTH );
    /** a link back from the C parser to this class*/
    this->m_http_parser_ptr->data = (void*) this;

    if (this->m_http_parser_settings_ptr != NULL) {
        free(this->m_http_parser_settings_ptr);
    }
    http_parser_settings* settings = (http_parser_settings*)eg_alloc(sizeof(http_parser_settings));
    this->m_http_parser_settings_ptr = settings;
    if(this->m_status_buf != NULL) CBuffer_clear(this->m_status_buf);
    if(this->m_url_buf != NULL) CBuffer_clear(this->m_url_buf);
    if(this->m_name_buf != NULL) CBuffer_clear(this->m_name_buf);
    if(this->m_value_buf != NULL) CBuffer_clear(this->m_value_buf);
    this->m_http_parser_settings_ptr->on_message_begin = message_begin_cb;
    this->m_http_parser_settings_ptr->on_url = url_data_cb;
    this->m_http_parser_settings_ptr->on_status = status_data_cb;
    this->m_http_parser_settings_ptr->on_header_field = header_field_data_cb;
    this->m_http_parser_settings_ptr->on_header_value = header_value_data_cb;
    this->m_http_parser_settings_ptr->on_headers_complete = headers_complete_cb;
    this->m_http_parser_settings_ptr->on_body = body_data_cb;
    this->m_http_parser_settings_ptr->on_message_complete = message_complete_cb;
    this->m_http_parser_settings_ptr->on_chunk_header = chunk_header_cb;
    this->m_http_parser_settings_ptr->on_chunk_complete = chunk_complete_cb;
}


int message_begin_cb(http_parser* parser)
{
    ParserRef this =  (ParserRef)(parser->data);
    if(this->m_status_buf != NULL) {
        CBuffer_clear(this->m_status_buf);
    } else {
        if((this->m_status_buf = CBuffer_new())==NULL)  return -1; // this will stop parsing and the error can be handled higher up
    }
    if(this->m_url_buf != NULL) {
        CBuffer_clear(this->m_url_buf);
    } else {
        if((this->m_url_buf = CBuffer_new()) == NULL) return -1; // this will stop parsing and the error can be handled higher up
    }
    if(this->m_name_buf != NULL) {
        CBuffer_clear(this->m_name_buf);
    } else {
        if((this->m_name_buf = CBuffer_new()) == NULL) return -1; // this will stop parsing and the error can be handled higher up
    }
    if(this->m_value_buf != NULL) {
        CBuffer_clear(this->m_value_buf);
    } else {
        if((this->m_value_buf = CBuffer_new()) == NULL)  return -1; // this will stop parsing and the error can be handled higher up
    }
    return 0;
}

int url_data_cb(http_parser* parser, const char* at, size_t length)
{
    ParserRef this =  (ParserRef)(parser->data);
    MessageRef message = Parser_current_message(this);
    Message_set_is_request(message, true);
    CBuffer_append(this->m_url_buf, (char*)at, length); /*NEEDS ALLO TEST*/
    return 0;
}

int status_data_cb(http_parser* parser, const char* at, size_t length)
{
    ParserRef this =  (ParserRef)(parser->data);
    MessageRef message = Parser_current_message(this);
    Message_set_is_request(message, false);
    Message_set_status(message, this->m_http_parser_ptr->status_code);

    CBuffer_append(this->m_status_buf, (char*)at, length);  /*NEEDS ALLO TEST*/
    Message_move_reason(message, this->m_status_buf);  /*NEEDS ALLO TEST*/
    return 0;
}
int header_field_data_cb(http_parser* parser, const char* at, size_t length)
{
    ParserRef this =  (ParserRef)(parser->data);
    MessageRef message = Parser_current_message(this);
    HDRListRef hdrs = Message_headers(message);
    int state = this->m_header_state;
    if( (state == 0) || (state == kHEADER_STATE_NOTHING) || (state == kHEADER_STATE_VALUE)) {
        if(CBuffer_size(this->m_name_buf) != 0) {
            HDRList_add(hdrs, this->m_name_buf, this->m_value_buf);  /*NEEDS ALLO TEST*/
            CBuffer_clear(this->m_name_buf);
            CBuffer_clear(this->m_value_buf);
        }
        CBuffer_append(this->m_name_buf, (char*)at, length);  /*NEEDS ALLO TEST*/
    } else if( state == kHEADER_STATE_FIELD ) {
        CBuffer_append(this->m_name_buf, (char*)at, length);  /*NEEDS ALLO TEST*/
    } else {
        assert(false);
    }
    this->m_header_state = kHEADER_STATE_FIELD;
    return 0;
}
int header_value_data_cb(http_parser* parser, const char* at, size_t length)
{
    ParserRef this =  (ParserRef)(parser->data);
    MessageRef message = Parser_current_message(this);
    int state = this->m_header_state;
    if( state == kHEADER_STATE_FIELD ) {
        CBuffer_clear(this->m_value_buf);
        CBuffer_append(this->m_value_buf, (char*)at, length);  /*NEEDS ALLO TEST*/
    } else if( state == kHEADER_STATE_VALUE) {
        CBuffer_append(this->m_value_buf, (char*)at, length);  /*NEEDS ALLO TEST*/
    } else {
        assert(false);
    }
    this->m_header_state = kHEADER_STATE_VALUE;
    return 0;
}
int headers_complete_cb(http_parser* parser) //, const char* aptr, size_t remainder)
{
    ParserRef this =  (ParserRef)(parser->data);
    MessageRef message = Parser_current_message(this);
    if( CBuffer_size(this->m_name_buf) != 0 ) {
        HDRList_add(Message_headers(message), this->m_name_buf, this->m_value_buf);  /*NEEDS ALLO TEST*/
        CBuffer_clear(this->m_name_buf);
        CBuffer_clear(this->m_value_buf);
    }
    Message_set_version(message, parser->http_major, parser->http_minor );
    if( CBuffer_size(this->m_url_buf)  == 0 ) {
    } else {
        Message_set_method(message, (enum http_method)parser->method);
        Message_move_target(message, this->m_url_buf);  /*NEEDS ALLO TEST*/
    }
//    if( CBuffer_size(this->m_status_buf) == 0 ) {
//    } else {
//        Message_move_reason(message, this->m_status_buf);
//    }
    this->m_header_done = true;
    return 0;
}
int body_data_cb(http_parser* parser, const char* at, size_t length)
{
    ParserRef this =  (ParserRef)(parser->data);
    MessageRef message = Parser_current_message(this);
    BufferChainRef chain_ptr = Message_get_body(message);
    if (chain_ptr == NULL) {
        chain_ptr = BufferChain_new();  /*NEEDS ALLO TEST*/
        Message_set_body(message, chain_ptr);
    }
    BufferChain_append(chain_ptr, (void*)at, length); /*NEEDS ALLO TEST*/
    return 0;
}
int chunk_header_cb(http_parser* parser)
{
    ParserRef p =  (ParserRef)(parser->data);
    return 0;
}
int chunk_complete_cb(http_parser* parser)
{
    ParserRef p =  (ParserRef)(parser->data);
    return 0;
}
int message_complete_cb(http_parser* parser)
{
    ParserRef this =  (ParserRef)(parser->data);
    this->m_message_done = true;
    
    // MessageBase* message = p->current_message();
    // p->OnMessageComplete(message);
    // force the parser to exit after this call
    // so that we dont process any data in the read
    // buffer beyond the end of the current message
    // in our application that is not possible but lets be careful
    // why not possible - only support one request on a connection
    // at a time - but multiple consecutive requests on the
    // same connection are possible. But a second request
    // wont be accepted until the first one is complete
    http_parser_pause(parser, 1); // TODO fix me
    /*
     * Now get ready for the next message
     */
    return 0;
}
