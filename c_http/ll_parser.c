#include <c_http/alloc.h>
#include <c_http/ll_parser.h>
#include <c_http/utils.h>
/**
 * @addtogroup http
 * @{
 */

/// forward declares 
static int chunk_header_cb(llhttp_t* parser);
static int chunk_complete_cb(llhttp_t* parser);
static int message_begin_cb(llhttp_t* parser);
static int url_data_cb(llhttp_t* parser, const char* at, size_t length);
static int status_data_cb(llhttp_t* parser, const char* at, size_t length);
static int header_field_data_cb(llhttp_t* parser, const char* at, size_t length);
static int header_value_data_cb(llhttp_t* parser, const char* at, size_t length);
static int headers_complete_cb(llhttp_t* parser);//, const char* aptr, size_t remainder);
static int body_data_cb(llhttp_t* parser, const char* at, size_t length);
static int message_complete_cb(llhttp_t* parser);

void Parser_initialize(ParserRef this);

ParserRef Parser_new()
{
    ParserRef this = eg_alloc(sizeof(Parser));
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

void Parser_free(ParserRef* this_p)
{
    ASSERT_NOT_NULL(*this_p);
    ParserRef this= *this_p;
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
MessageRef Parser_current_message(ParserRef this)
{
    return this->m_current_message_ptr;
}
int Parser_append_bytes(ParserRef this, void *buffer, unsigned length)
{
    // @TODO - need to handle error
    llhttp_errno_t errno  = llhttp_execute(this->m_llhttp_ptr, (const char*)buffer, (int)length);
    size_t nparsed = (unsigned long)llhttp_get_error_pos(this->m_llhttp_ptr) - (unsigned long)buffer;
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
    llhttp_errno_t errno = HPE_OK;
    if (length == 0) {
        errno = llhttp_finish(this->m_llhttp_ptr);
    } else {
        errno = llhttp_execute(this->m_llhttp_ptr, b, length);
    }
    int need_eof = llhttp_message_needs_eof(this->m_llhttp_ptr);
    int nparsed;
    if (errno == HPE_OK) {
        nparsed = length;
    } else if ((errno == HPE_PAUSED) && (length == 0)) {
        nparsed = 0;
    } else {
        nparsed = llhttp_get_error_pos(this->m_llhttp_ptr) - b;
    }
    rv.bytes_remaining = length - nparsed;
    if ((errno == HPE_OK) || (errno == HPE_PAUSED)) {
        // only possibilities - processed entire buffer or on_message_complete paused parser before buffer fully consumed
        if (this->m_message_done) {
            rv.return_code = ParserRC_end_of_message;
        } else if (this->m_header_done) {
            assert(nparsed == length); // should only get here if buffer finished exactly at end of headers
            rv.return_code = ParserRC_end_of_header;
        } else if (nparsed == length) {
            rv.return_code = ParserRC_end_of_data;
        } else {
            assert(false); // should never get here
        }
    } else {
        rv.return_code = ParserRC_error;
        ParserError x = Parser_get_error(this);
    }
    return rv;
}
llhttp_errno_t Parser_get_errno(ParserRef this)
{
    llhttp_errno_t x = llhttp_get_errno(this->m_llhttp_ptr);
    return x;
}
ParserError Parser_get_error(ParserRef this)
{
    llhttp_errno_t x = llhttp_get_errno(this->m_llhttp_ptr);
    char* n = (char*)llhttp_errno_name(x);
    char* d = (char*)llhttp_errno_name(x);
    ParserError erst;
    erst.m_err_number = x;
    erst.m_name = n;
    erst.m_description = d;
    return erst;

}

void Parser_initialize(ParserRef this)
{
    this->m_header_state = kHEADER_STATE_NOTHING;
    this->m_started = false;
    this->m_message_done = false;
    this->m_header_done = false;
    this->m_current_message_ptr = NULL;

    if (this->m_llhttp_settings_ptr != NULL) {
        free(this->m_llhttp_settings_ptr);
    }
    llhttp_settings_t* settings = (llhttp_settings_t*)eg_alloc(sizeof(llhttp_settings_t));
    this->m_llhttp_settings_ptr = settings;
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

    if (this->m_llhttp_ptr != NULL) {
        free(this->m_llhttp_ptr);
        this->m_llhttp_ptr = NULL;
    }
    this->m_llhttp_ptr = (llhttp_t*)eg_alloc(sizeof(llhttp_t));
    llhttp_init( this->m_llhttp_ptr, HTTP_BOTH, settings);
    /** a link back from the C parser to this class*/
    this->m_llhttp_ptr->data = (void*) this;

}

static
int message_begin_cb(llhttp_t* parser)
{
    ParserRef this =  (ParserRef)(parser->data);
    MessageRef message = Parser_current_message(this);
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
    ParserRef this =  (ParserRef)(parser->data);
    MessageRef message = Parser_current_message(this);
    Message_set_is_request(message, true);
    Cbuffer_append(this->m_url_buf, (char*)at, length); /*NEEDS ALLO TEST*/
    return 0;
}

static
int status_data_cb(llhttp_t* parser, const char* at, size_t length)
{
    ParserRef this =  (ParserRef)(parser->data);
    MessageRef message = Parser_current_message(this);
    Message_set_is_request(message, false);
    Message_set_status(message, this->m_llhttp_ptr->status_code);

    Cbuffer_append(this->m_status_buf, (char*)at, length);  /*NEEDS ALLO TEST*/
    Message_set_reason_cbuffer(message, this->m_status_buf);  /*NEEDS ALLO TEST*/
    return 0;
}
static
int header_field_data_cb(llhttp_t* parser, const char* at, size_t length)
{
    ParserRef this =  (ParserRef)(parser->data);
    MessageRef message = Parser_current_message(this);
    HdrListRef hdrs = Message_get_headerlist(message);
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
    ParserRef this =  (ParserRef)(parser->data);
    MessageRef message = Parser_current_message(this);
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
    ParserRef this =  (ParserRef)(parser->data);
    MessageRef message = Parser_current_message(this);
    if( Cbuffer_size(this->m_name_buf) != 0 ) {
        HdrList_add_cbuf(Message_get_headerlist(message), this->m_name_buf, this->m_value_buf);  /*NEEDS ALLO TEST*/
        Cbuffer_clear(this->m_name_buf);
        Cbuffer_clear(this->m_value_buf);
    }
    Message_set_version(message, parser->http_major, parser->http_minor );
    if( Cbuffer_size(this->m_url_buf)  == 0 ) {
    } else {
        Message_set_method(message, (llhttp_method_t)parser->method);
        Message_set_target_cbuffer(message, this->m_url_buf);  /*NEEDS ALLO TEST*/
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
static
int chunk_header_cb(llhttp_t* parser)
{
    ParserRef this =  (ParserRef)(parser->data);
    MessageRef message = Parser_current_message(this);
    return 0;
}
static
int chunk_complete_cb(llhttp_t* parser)
{
    ParserRef p =  (ParserRef)(parser->data);
    return 0;
}
static
int message_complete_cb(llhttp_t* parser)
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
    // llhttp_parser_pause(parser, 1); // TODO fix me
    /*
     * Now get ready for the next message
     */
    return HPE_PAUSED;
}
/**@}*/