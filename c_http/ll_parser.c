#include <c_http/alloc.h>
#include <c_http/ll_parser.h>
#include <c_http/utils.h>

/// forward declares 
int chunk_header_cb(llhttp_t* parser);
int chunk_complete_cb(llhttp_t* parser);
int message_begin_cb(llhttp_t* parser);
int url_data_cb(llhttp_t* parser, const char* at, size_t length);
int status_data_cb(llhttp_t* parser, const char* at, size_t length);
int header_field_data_cb(llhttp_t* parser, const char* at, size_t length);
int header_value_data_cb(llhttp_t* parser, const char* at, size_t length);
int headers_complete_cb(llhttp_t* parser);//, const char* aptr, size_t remainder);
int body_data_cb(llhttp_t* parser, const char* at, size_t length);
int message_complete_cb(llhttp_t* parser);

void LL_Parser_initialize(ParserRef this);

LL_ParserRef LL_Parser_new()
{
    LL_ParserRef this = eg_alloc(sizeof(Parser));
    if(this == NULL)
        return NULL;
    this->m_message_done = false;
    this->m_header_done = false;
    this->m_llhttp_ptr = NULL;
    this->m_llhttp_settings_ptr = NULL;
    this->m_header_state = kHEADER_STATE_NOTHING;
    this->m_status_buf = Cbuffer_new();
    this->m_url_buf    = Cbuffer_new();
    this->m_name_buf   = Cbuffer_new();
    this->m_value_buf  = Cbuffer_new();
    return this;
}

void LL_Parser_free(LL_ParserRef* this_p)
{
    ASSERT_NOT_NULL(*this_p);
    LL_ParserRef this= *this_p;
    if (this->m_llhttp_ptr != NULL) {
        free(this->m_llhttp_ptr);
        this->m_llhttp_ptr = NULL;
    }
    if (this->m_llhttp_settings_ptr != NULL) {
        free(this->m_llhttp_settings_ptr);
        this->m_llhttp_settings_ptr = NULL;
    }

    if(this->m_name_buf != NULL) Cbuffer_free(&(this->m_name_buf));
    if(this->m_url_buf != NULL) Cbuffer_free(&(this->m_url_buf));
    if(this->m_status_buf != NULL) Cbuffer_free(&(this->m_status_buf));
    if(this->m_value_buf != NULL) Cbuffer_free(&(this->m_value_buf));
    if(this->m_name_buf != NULL) Cbuffer_free(&(this->m_name_buf));
    free(this);
    *this_p = NULL;

}
MessageRef LL_Parser_current_message(LL_ParserRef this)
{
    return this->m_current_message_ptr;
}
int LL_Parser_append_bytes(LL_ParserRef this, void *buffer, unsigned length)
{
    // @TODO - need to handle error
    llhttp_errno errno  = llhttp_execute(this->m_llhttp_ptr, (const char*)buffer, (int)length);
    size_t nparsed = llhttp_get_error_pos(this->m_llhttp_ptr) - buffer;
    return (int)nparsed;
}
void LL_Parser_begin(LL_ParserRef this, MessageRef message_ptr)
{
    LL_Parser_initialize(this);
    this->m_current_message_ptr = message_ptr;
}

LL_ParserReturnValue LL_Parser_consume(LL_ParserRef this, const void* buf, int length)
{
    bool only_header = false;
    this->m_started = true;
    LL_ParserReturnValue rv = {.return_code = ParserRC_end_of_data, .bytes_remaining = length};
    char* b = (char*) buf;
    size_t total_parsed = 0;
    char* b_start_ptr = &(b[total_parsed]);
    llhttp_errno errno = llhttp_execute(this->m_llhttp_ptr, b_start_ptr, length - total_parsed);
    int nparsed = llhttp_get_error_pos(this->m_llhttp_ptr) - b_start_ptr;
    total_parsed = total_parsed + nparsed;
    rv.bytes_remaining = length - nparsed;
    if (LL_Parser_is_error(this)) {
        rv.return_code = ParserRC_error;
        LL_ParserError x = LL_Parser_get_error(this);
        return rv;
    } else if (this->m_message_done) {
        rv.return_code = LL_ParserRC_end_of_message;
        return rv;
    } else if (this->m_header_done) {
        rv.return_code = LL_ParserRC_end_of_header;
        if (only_header) {
            return rv;
        }
    } else if (nparsed == length) {

    }
    return rv;
}
#ifdef NOIMPLEMENTED

LL_ParserReturnValue LL_Parser_end(LL_ParserRef this)
{
    assert(false);
    LL_ParserReturnValue rv = {.return_code = ParserRC_end_of_data, .bytes_remaining = 0};
    char* buffer = NULL;
    size_t nparsed;
    llhttp_errno p_error;
    int someLength = 0;
    if( ! this->m_message_done ) {
        if (this->m_started) {
            p_error = llhttp_execute(this->m_llhttp_ptr, buffer, someLength);
        }
        if (LL_Parser_is_error(this)) {
            rv.return_code = LL_ParserRC_error;
            ParserError x = LL_Parser_get_error(this);
            return rv;
        } else if (this->m_message_done) {
            rv.return_code = LL_ParserRC_end_of_message;
            return rv;
        } else if (this->m_header_done) {
            rv.return_code = LL_ParserRC_end_of_header;
        } else {
            printf("should not be here\n");
            return rv;
        }
        return rv;
    }
    return rv;
}
void LL_Parser_append_eof(LL_ParserRef this)
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
#endif
enum http_errno LL_Parser_get_errno(LL_ParserRef this)
{
    llhttp_errno_t x = llhttp_get_errno(this->m_llhttp_ptr);
    return x;
}
LL_ParserError LL_Parser_get_error(LL_ParserRef this)
{
    llhttp_errno_t x = llhttp_get_errno(this->m_llhttp_ptr);
    char* n = (char*)llhttp_errno_name(x);
    char* d = (char*)llhttp_errno_name(x);
    LL_ParserError erst;
    erst.m_err_number = x;
    erst.m_name = n;
    erst.m_description = d;
    return erst;

}
bool LL_Parser_is_error(LL_ParserRef this)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wall"
    llhttp_errno_t x = llhttp_get_errno(this->m_llhttp_ptr);
    char* n = (char*)llhttp_errno_name(x);
    char* d = (char*)llhttp_errno_name(x);
#pragma clang diagnostic pops
    return (x != HPE_OK) && (x != HPE_PAUSED);
};

void LL_Parser_initialize(LL_ParserRef this)
{
    this->m_header_state = kHEADER_STATE_NOTHING;
    this->m_started = false;
    this->m_message_done = false;
    this->m_header_done = false;
    this->m_current_message_ptr = NULL;
    if (this->m_http_parser_ptr != NULL) {
        free(this->m_http_parser_ptr);
    }
    this->m_http_parser_ptr = (llhttp_t*)eg_alloc(sizeof(llhttp_t));
    http_parser_init( this->m_http_parser_ptr, HTTP_BOTH );
    /** a link back from the C parser to this class*/
    this->m_http_parser_ptr->data = (void*) this;

    if (this->m_http_parser_settings_ptr != NULL) {
        free(this->m_http_parser_settings_ptr);
    }
    llhttp_settings* settings = (llhttp_settings*)eg_alloc(sizeof(llhttp_settings_s));
    this->m_http_parser_settings_ptr = settings;
    if(this->m_status_buf != NULL) Cbuffer_clear(this->m_status_buf);
    if(this->m_url_buf != NULL) Cbuffer_clear(this->m_url_buf);
    if(this->m_name_buf != NULL) Cbuffer_clear(this->m_name_buf);
    if(this->m_value_buf != NULL) Cbuffer_clear(this->m_value_buf);
    this->m_llhttp_settings_ptr->on_message_begin = message_begin_cb;
    this->m_llhttp_settings_ptr->on_url = url_data_cb;
    this->m_llhttp_settings_ptr->on_status = status_data_cb;
    this->m_llhttp_settings_ptr->on_header_field = header_field_data_cb;
    this->m_llhttp_settings_ptr->on_header_value = header_value_data_cb;
    this->m_llhttp_settings_ptr->on_headers_complete = headers_complete_cb;
    this->m_llhttp_settings_ptr->on_body = body_data_cb;
    this->m_llhttp_settings_ptr->on_message_complete = message_complete_cb;
    this->m_llhttp_settings_ptr->on_chunk_header = chunk_header_cb;
    this->m_llhttp_settings_ptr->on_chunk_complete = chunk_complete_cb;
}

static
int message_begin_cb(llhttp_t* parser)
{
    LL_ParserRef this =  (LL_ParserRef)(parser->data);
    MessageRef message = LL_Parser_current_message(this);
    if(this->m_status_buf != NULL) {
        Cbuffer_clear(this->m_status_buf);
    } else {
        if((this->m_status_buf = Cbuffer_new())==NULL)  return -1; // this will stop parsing and the error can be handled higher up
    }
    if(this->m_url_buf != NULL) {
        Cbuffer_clear(this->m_url_buf);
    } else {
        if((this->m_url_buf = Cbuffer_new()) == NULL) return -1; // this will stop parsing and the error can be handled higher up
    }
    if(this->m_name_buf != NULL) {
        Cbuffer_clear(this->m_name_buf);
    } else {
        if((this->m_name_buf = Cbuffer_new()) == NULL) return -1; // this will stop parsing and the error can be handled higher up
    }
    if(this->m_value_buf != NULL) {
        Cbuffer_clear(this->m_value_buf);
    } else {
        if((this->m_value_buf = Cbuffer_new()) == NULL)  return -1; // this will stop parsing and the error can be handled higher up
    }
    return 0;
}

static
int url_data_cb(llhttp_t* parser, const char* at, size_t length)
{
    LL_ParserRef this =  (LL_ParserRef)(parser->data);
    MessageRef message = LL_Parser_current_message(this);
    Message_set_is_request(message, true);
    Cbuffer_append(this->m_url_buf, (char*)at, length); /*NEEDS ALLO TEST*/
    return 0;
}

static
int status_data_cb(llhttp_t* parser, const char* at, size_t length)
{
    LL_ParserRef this =  (LL_ParserRef)(parser->data);
    MessageRef message = LL_Parser_current_message(this);
    Message_set_is_request(message, false);
    Message_set_status(message, this->m_http_parser_ptr->status_code);

    Cbuffer_append(this->m_status_buf, (char*)at, length);  /*NEEDS ALLO TEST*/
    Message_move_reason(message, this->m_status_buf);  /*NEEDS ALLO TEST*/
    return 0;
}
static
int header_field_data_cb(llhttp_t* parser, const char* at, size_t length)
{
    LL_ParserRef this =  (LL_ParserRef)(parser->data);
    MessageRef message = LL_Parser_current_message(this);
    HdrListRef hdrs = Message_headers(message);
    int state = this->m_header_state;
    if( (state == 0) || (state == kHEADER_STATE_NOTHING) || (state == kHEADER_STATE_VALUE)) {
        if(Cbuffer_size(this->m_name_buf) != 0) {
            HdrList_add_cbuf(hdrs, this->m_name_buf, this->m_value_buf);  /*NEEDS ALLO TEST*/
            Cbuffer_clear(this->m_name_buf);
            Cbuffer_clear(this->m_value_buf);
        }
        Cbuffer_append(this->m_name_buf, (char*)at, length);  /*NEEDS ALLO TEST*/
    } else if( state == kHEADER_STATE_FIELD ) {
        Cbuffer_append(this->m_name_buf, (char*)at, length);  /*NEEDS ALLO TEST*/
    } else {
        assert(false);
    }
    this->m_header_state = kHEADER_STATE_FIELD;
    return 0;
}
static
int header_value_data_cb(llhttp_t* parser, const char* at, size_t length)
{
    LL_ParserRef this =  (LL_ParserRef)(parser->data);
    MessageRef message = LL_Parser_current_message(this);
    int state = this->m_header_state;
    if( state == kHEADER_STATE_FIELD ) {
        Cbuffer_clear(this->m_value_buf);
        Cbuffer_append(this->m_value_buf, (char*)at, length);  /*NEEDS ALLO TEST*/
    } else if( state == kHEADER_STATE_VALUE) {
        Cbuffer_append(this->m_value_buf, (char*)at, length);  /*NEEDS ALLO TEST*/
    } else {
        assert(false);
    }
    this->m_header_state = kHEADER_STATE_VALUE;
    return 0;
}
static
int headers_complete_cb(llhttp_t* parser) //, const char* aptr, size_t remainder)
{
    LL_ParserRef this =  (LL_ParserRef)(parser->data);
    MessageRef message = LL_Parser_current_message(this);
    if( Cbuffer_size(this->m_name_buf) != 0 ) {
        HdrList_add_cbuf(Message_headers(message), this->m_name_buf, this->m_value_buf);  /*NEEDS ALLO TEST*/
        Cbuffer_clear(this->m_name_buf);
        Cbuffer_clear(this->m_value_buf);
    }
    Message_set_version(message, parser->http_major, parser->http_minor );
    if( Cbuffer_size(this->m_url_buf)  == 0 ) {
    } else {
        Message_set_method(message, (enum http_method)parser->method);
        Message_move_target(message, this->m_url_buf);  /*NEEDS ALLO TEST*/
    }
//    if( Cbuffer_size(this->m_status_buf) == 0 ) {
//    } else {
//        Message_move_reason(message, this->m_status_buf);
//    }
    this->m_header_done = true;
    return 0;
}
static
int body_data_cb(llhttp_t* parser, const char* at, size_t length)
{
    LL_ParserRef this =  (LL_ParserRef)(parser->data);
    MessageRef message = LL_Parser_current_message(this);
    BufferChainRef chain_ptr = Message_get_body(message);
    if (chain_ptr == NULL) {
        chain_ptr = BufferChain_new();  /*NEEDS ALLO TEST*/
        Message_set_body(message, chain_ptr);
    }
    BufferChain_append(chain_ptr, (void*)at, length); /*NEEDS ALLO TEST*/
    return 0;
}
static
int chunk_header_cb(llhttp_t* parser)
{
    LL_ParserRef this =  (LL_ParserRef)(parser->data);
    MessageRef message = LL_Parser_current_message(this);
    return 0;
}
static
int chunk_complete_cb(llhttp_t* parser)
{
    LL_ParserRef p =  (LL_ParserRef)(parser->data);
    return 0;
}
static
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
