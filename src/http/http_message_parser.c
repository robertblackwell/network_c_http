#include <src/common/alloc.h>
#include "http_message_parser.h"
#include <src/common/utils.h>
/// forward declares
static int message_begin_cb(llhttp_t* parser);
static int url_data_cb(llhttp_t* parser, const char* at, size_t length);
static int status_data_cb(llhttp_t* parser, const char* at, size_t length);
static int method_data_cb(llhttp_t* parser, const char* at, size_t length);
static int version_data_cb(llhttp_t* parser, const char* at, size_t length);
static int header_field_data_cb(llhttp_t* parser, const char* at, size_t length);
static int header_value_data_cb(llhttp_t* parser, const char* at, size_t length);
static int chunk_extension_name_cb(llhttp_t* parser, const char* at, size_t length);
static int chunk_extension_value_cb(llhttp_t* parser, const char* at, size_t length);

static int headers_complete_cb(llhttp_t* parser);//, const char* aptr, size_t remainder);
static int body_data_cb(llhttp_t* parser, const char* at, size_t length);
static int message_complete_cb(llhttp_t* parser);

static int on_url_complete_cb(llhttp_t* parser);
static int on_status_complete_cb(llhttp_t* parser);
static int on_method_complete_cb(llhttp_t* parser);
static int on_version_complete_cb(llhttp_t* parser);
static int on_header_field_complete_cb(llhttp_t* parser);
static int on_header_value_complete_cb(llhttp_t* parser);
static int on_chunk_extension_name_complete_cb(llhttp_t* parser);
static int on_chunk_extension_value_complete_cb(llhttp_t* parser);

static int chunk_header_cb(llhttp_t* parser);
static int chunk_complete_cb(llhttp_t* parser);
static int on_reset_cb(llhttp_t* parser);

void HttpParser_initialize(HttpMessageParser *this);
HttpMessageParserRef http_message_parser_new(void(*on_new_message_cb)(void* ctx, HttpMessageRef new_msg_ref, int error), void* handler_context)
{
    HttpMessageParserRef this = eg_alloc(sizeof(HttpMessageParser));
    if(this == NULL)
        return NULL;
    RBL_SET_TAG(HTTP_PARSER_TAG, this)
    this->m_llhttp_ptr = NULL;
    this->m_llhttp_settings_ptr = NULL;
    this->m_header_state = kHEADER_STATE_NOTHING;
    this->m_status_buf = Cbuffer_new();
    this->m_url_buf    = Cbuffer_new();
    this->m_name_buf   = Cbuffer_new();
    this->m_value_buf  = Cbuffer_new();
    HttpParser_initialize(this);
    this->on_message_handler = on_new_message_cb;
    this->on_message_handler_context = handler_context;
    return this;
}
void HttpParser_reset(HttpMessageParser* parser_ptr)
{

}
void http_message_parser_free(HttpMessageParserRef this)
{
    ASSERT_NOT_NULL(this);
    RBL_CHECK_TAG(HTTP_PARSER_TAG, this)
    if (this->m_llhttp_ptr != NULL) {
        free(this->m_llhttp_ptr);
        this->m_llhttp_ptr = NULL;
    }
    if (this->m_llhttp_settings_ptr != NULL) {
        free(this->m_llhttp_settings_ptr);
        this->m_llhttp_settings_ptr = NULL;
    }

    if(this->m_name_buf != NULL) Cbuffer_free(this->m_name_buf);
    if(this->m_url_buf != NULL) Cbuffer_free(this->m_url_buf);
    if(this->m_status_buf != NULL) Cbuffer_free(this->m_status_buf);
    if(this->m_value_buf != NULL) Cbuffer_free(this->m_value_buf);
    free(this);
}
int Parser_append_bytes(HttpMessageParserRef this, void *buffer, unsigned length)
{
    // @TODO - need to handle error
    RBL_CHECK_TAG(HTTP_PARSER_TAG, this)
    llhttp_errno_t errno  = llhttp_execute(this->m_llhttp_ptr, (const char*)buffer, (int)length);
    size_t nparsed = (unsigned long)llhttp_get_error_pos(this->m_llhttp_ptr) - (unsigned long)buffer;
    return (int)nparsed;
}

llhttp_errno_t http_message_parser_consume(HttpMessageParser *parser, const void* buffer, int length)
{
    RBL_CHECK_TAG(HTTP_PARSER_TAG, parser)
    char* b = (char*) buffer;
    int need_eof = llhttp_message_needs_eof(parser->m_llhttp_ptr);
    llhttp_errno_t x = http_message_parser_get_errno(parser);
    llhttp_errno_t llerrno = HPE_OK;
    if ((length == 0) && (llhttp_message_needs_eof(parser->m_llhttp_ptr))) {
        llerrno = llhttp_finish(parser->m_llhttp_ptr);
    } else {
        llerrno = llhttp_execute(parser->m_llhttp_ptr, b, length);
    }
    return llerrno;
}
llhttp_errno_t  http_message_parser_consume_buffer(HttpMessageParser *parser, IOBufferRef iobuffer_ref)
{
    llhttp_errno_t llerrno;
    void* bufptr = IOBuffer_data(iobuffer_ref);
    int buflength = IOBuffer_data_len(iobuffer_ref);
    if ((buflength == 0) && (llhttp_message_needs_eof(parser->m_llhttp_ptr))) {
        llerrno = llhttp_finish(parser->m_llhttp_ptr);
    } else {
        llerrno = llhttp_execute(parser->m_llhttp_ptr, bufptr, buflength);
    }
    if(llerrno == HPE_OK) {
        IOBuffer_consume(iobuffer_ref, buflength);
    }
    return llerrno;

}
llhttp_errno_t  http_message_parser_consume_eof(HttpMessageParser* parser)
{
    llhttp_errno_t llerrno = HPE_OK;
    if(llhttp_message_needs_eof(parser->m_llhttp_ptr)) {
        llerrno = llhttp_finish(parser->m_llhttp_ptr);
    }
    return llerrno;
}

llhttp_errno_t http_message_parser_get_errno(HttpMessageParser* parser)
{
    RBL_CHECK_TAG(HTTP_PARSER_TAG, parser)

    llhttp_errno_t x = llhttp_get_errno(parser->m_llhttp_ptr);
    return x;
}
const void* http_message_parser_last_byte_parsed(HttpMessageParser* this)
{
    RBL_CHECK_TAG(HTTP_PARSER_TAG, this)
    const void* x = llhttp_get_error_pos(this->m_llhttp_ptr);
    return x;
}
http_parser_error_t http_message_parser_get_error(HttpMessageParser *parser)
{
    RBL_CHECK_TAG(HTTP_PARSER_TAG, parser)
    llhttp_errno_t x = llhttp_get_errno(parser->m_llhttp_ptr);
    char* n = (char*)llhttp_errno_name(x);
    char* d = (char*)llhttp_errno_name(x);
    http_parser_error_t erst;
    erst.m_err_number = x;
    erst.m_name = n;
    erst.m_description = d;
    return erst;

}

void HttpParser_initialize(HttpMessageParser* this)
{
    RBL_CHECK_TAG(HTTP_PARSER_TAG, this)

    this->m_header_state = kHEADER_STATE_NOTHING;
    this->m_started = false;
    this->current_message_ptr = http_message_new();
    this->current_message_ptr = NULL;

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
    this->m_llhttp_settings_ptr->on_method = method_data_cb;
    this->m_llhttp_settings_ptr->on_version = version_data_cb;
    this->m_llhttp_settings_ptr->on_header_field = header_field_data_cb;
    this->m_llhttp_settings_ptr->on_header_value = header_value_data_cb;
    this->m_llhttp_settings_ptr->on_chunk_extension_name = chunk_extension_name_cb;
    this->m_llhttp_settings_ptr->on_chunk_extension_value = chunk_extension_value_cb;

    this->m_llhttp_settings_ptr->on_headers_complete = headers_complete_cb;

    this->m_llhttp_settings_ptr->on_body = body_data_cb;

    this->m_llhttp_settings_ptr->on_message_complete = message_complete_cb;
    this->m_llhttp_settings_ptr->on_url_complete = on_url_complete_cb;
    this->m_llhttp_settings_ptr->on_status_complete = on_status_complete_cb;
    this->m_llhttp_settings_ptr->on_method_complete = on_method_complete_cb;
    this->m_llhttp_settings_ptr->on_version_complete = on_version_complete_cb;
    this->m_llhttp_settings_ptr->on_header_field_complete = on_header_field_complete_cb;
    this->m_llhttp_settings_ptr->on_header_value_complete = on_header_value_complete_cb;
    this->m_llhttp_settings_ptr->on_chunk_extension_name_complete = on_chunk_extension_name_complete_cb;
    this->m_llhttp_settings_ptr->on_chunk_extension_value_complete = on_chunk_extension_value_complete_cb;

    this->m_llhttp_settings_ptr->on_chunk_header = chunk_header_cb;
    this->m_llhttp_settings_ptr->on_chunk_complete = chunk_complete_cb;
    this->m_llhttp_settings_ptr->on_reset = on_reset_cb;


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
    HttpMessageParserRef this =  (HttpMessageParserRef)(parser->data);
    RBL_CHECK_TAG(HTTP_PARSER_TAG, this)

    if(this->current_message_ptr != NULL) {
        http_message_free(this->current_message_ptr);
        this->current_message_ptr = http_message_new();
    } else {
        this->current_message_ptr = http_message_new();
    }
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
    HttpMessageParserRef this =  (HttpMessageParserRef)(parser->data);
    RBL_CHECK_TAG(HTTP_PARSER_TAG, this)
    http_message_set_is_request(this->current_message_ptr, true);
    Cbuffer_append(this->m_url_buf, (char*)at, length); /*NEEDS ALLO TEST*/
    return 0;
}

static
int status_data_cb(llhttp_t* parser, const char* at, size_t length)
{
    HttpMessageParserRef this =  (HttpMessageParserRef)(parser->data);
    RBL_CHECK_TAG(HTTP_PARSER_TAG, this)
    http_message_set_is_request(this->current_message_ptr, false);
    http_message_set_status(this->current_message_ptr, this->m_llhttp_ptr->status_code);

    Cbuffer_append(this->m_status_buf, (char*)at, length);  /*NEEDS ALLO TEST*/
    http_message_set_reason_cbuffer(this->current_message_ptr, this->m_status_buf);  /*NEEDS ALLO TEST*/
    return 0;
}
static
int method_data_cb(llhttp_t* parser, const char* at, size_t length)
{
    return 0;
    HttpMessageParserRef this =  (HttpMessageParserRef)(parser->data);
    http_message_set_is_request(this->current_message_ptr, false);
    http_message_set_status(this->current_message_ptr, this->m_llhttp_ptr->status_code);

    Cbuffer_append(this->m_status_buf, (char*)at, length);  /*NEEDS ALLO TEST*/
    http_message_set_reason_cbuffer(this->current_message_ptr, this->m_status_buf);  /*NEEDS ALLO TEST*/
    return 0;
}
static
int version_data_cb(llhttp_t* parser, const char* at, size_t length)
{
    return 0;
    HttpMessageParserRef this =  (HttpMessageParserRef)(parser->data);
    http_message_set_is_request(this->current_message_ptr, false);
    http_message_set_status(this->current_message_ptr, this->m_llhttp_ptr->status_code);

    Cbuffer_append(this->m_status_buf, (char*)at, length);  /*NEEDS ALLO TEST*/
    http_message_set_reason_cbuffer(this->current_message_ptr, this->m_status_buf);  /*NEEDS ALLO TEST*/
    return 0;
}

static
int header_field_data_cb(llhttp_t* parser, const char* at, size_t length)
{
    HttpMessageParserRef this =  (HttpMessageParserRef)(parser->data);
    RBL_CHECK_TAG(HTTP_PARSER_TAG, this)
    int state = this->m_header_state;
    if( (state == 0) || (state == kHEADER_STATE_NOTHING) || (state == kHEADER_STATE_VALUE)) {
        if(Cbuffer_size(this->m_name_buf) != 0) {
            http_message_add_header_cbuf(this->current_message_ptr, this->m_name_buf, this->m_value_buf);
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
    HttpMessageParserRef this =  (HttpMessageParserRef)(parser->data);
    RBL_CHECK_TAG(HTTP_PARSER_TAG, this)
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
int chunk_extension_name_cb(llhttp_t* parser, const char* at, size_t length)
{
    return 0;
    HttpMessageParserRef this =  (HttpMessageParserRef)(parser->data);
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
int chunk_extension_value_cb(llhttp_t* parser, const char* at, size_t length)
{
    return 0;
    HttpMessageParserRef this =  (HttpMessageParserRef)(parser->data);
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
    HttpMessageParserRef this =  (HttpMessageParserRef)(parser->data);
    RBL_CHECK_TAG(HTTP_PARSER_TAG, this)
    if( Cbuffer_size(this->m_name_buf) != 0 ) {
        http_message_add_header_cbuf(this->current_message_ptr, this->m_name_buf, this->m_value_buf);
        Cbuffer_clear(this->m_name_buf);
        Cbuffer_clear(this->m_value_buf);
    }
    http_message_set_version(this->current_message_ptr, parser->http_major, parser->http_minor);
    if( Cbuffer_size(this->m_url_buf)  == 0 ) {
    } else {
        http_message_set_method(this->current_message_ptr, (llhttp_method_t) parser->method);
        http_message_set_target_cbuffer(this->current_message_ptr, this->m_url_buf);  /*NEEDS ALLO TEST*/
    }
//    if( Cbuffer_size(this->m_status_buf) == 0 ) {
//    } else {
//        Message_move_reason(message, this->m_status_buf);
//    }
//    this->m_header_done = true;
    return 0;
}
static
int body_data_cb(llhttp_t* parser, const char* at, size_t length)
{
    HttpMessageParserRef this =  (HttpMessageParserRef)(parser->data);
    RBL_CHECK_TAG(HTTP_PARSER_TAG, this)
    BufferChainRef chain_ptr = http_message_get_body(this->current_message_ptr);
    if (chain_ptr == NULL) {
        chain_ptr = BufferChain_new();  /*NEEDS ALLO TEST*/
        http_message_set_body(this->current_message_ptr, chain_ptr);
    }
    BufferChain_append(chain_ptr, (void*)at, length); /*NEEDS ALLO TEST*/
    return 0;
}

static
int message_complete_cb(llhttp_t* parser)
{
    HttpMessageParserRef this =  (HttpMessageParserRef)(parser->data);
    RBL_CHECK_TAG(HTTP_PARSER_TAG, this)
    HttpMessageRef tmp = this->current_message_ptr;
    this->current_message_ptr = http_message_new();
    this->on_message_handler(this->on_message_handler_context, tmp, 0);
    return 0;
}
static
int on_url_complete_cb(llhttp_t* parser)
{
    return 0;
}
static
int on_status_complete_cb(llhttp_t* parser)
{
    return 0;
}
static
int on_method_complete_cb(llhttp_t* parser)
{
    return 0;
}
static
int on_version_complete_cb(llhttp_t* parser)
{
    return 0;
}
static
int on_header_field_complete_cb(llhttp_t* parser)
{
    return 0;
}
static
int on_header_value_complete_cb(llhttp_t* parser)
{
    return 0;
}
static
int on_chunk_extension_name_complete_cb(llhttp_t* parser)
{
    return 0;
}
static
int on_chunk_extension_value_complete_cb(llhttp_t* parser)
{
    return 0;
}

static
int chunk_header_cb(llhttp_t* parser)
{
    HttpMessageParserRef this =  (HttpMessageParserRef)(parser->data);
    return 0;
}
static
int chunk_complete_cb(llhttp_t* parser)
{
    HttpMessageParserRef p =  (HttpMessageParserRef)(parser->data);
    return 0;
}
static
int on_reset_cb(llhttp_t* parser)
{
    HttpMessageParserRef p =  (HttpMessageParserRef)(parser->data);
    return 0;
}
