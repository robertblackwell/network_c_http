#include <http_in_c/common/alloc.h>
#include <http_in_c/http/parser.h>
#include <http_in_c/common/utils.h>
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

void http_parser_initialize(http_parser_r this);
http_parser_r http_parser_new(ParserOnMessageCompleteHandler handler, void* handler_context)
{
    http_parser_r this = eg_alloc(sizeof(http_parser_t));
    if(this == NULL)
        return NULL;
    SET_TAG(HTTP_PARSER_TAG, this)
    this->m_llhttp_ptr = NULL;
    this->m_llhttp_settings_ptr = NULL;
    this->m_header_state = kHEADER_STATE_NOTHING;
    this->m_status_buf = Cbuffer_new();
    this->m_url_buf    = Cbuffer_new();
    this->m_name_buf   = Cbuffer_new();
    this->m_value_buf  = Cbuffer_new();
    http_parser_initialize(this);
    this->on_message_handler = handler;
    this->handler_context = handler_context;
    return this;
}
void http_parser_reset(http_parser_t* this)
{

}
void http_parser_dispose(http_parser_r* parser_p)
{
    ASSERT_NOT_NULL(*parser_p);
    CHECK_TAG(HTTP_PARSER_TAG, *parser_p)
    http_parser_r this= *parser_p;
    if (this->m_llhttp_ptr != NULL) {
        free(this->m_llhttp_ptr);
        this->m_llhttp_ptr = NULL;
    }
    if (this->m_llhttp_settings_ptr != NULL) {
        free(this->m_llhttp_settings_ptr);
        this->m_llhttp_settings_ptr = NULL;
    }

    if(this->m_name_buf != NULL) Cbuffer_dispose(&(this->m_name_buf));
    if(this->m_url_buf != NULL) Cbuffer_dispose(&(this->m_url_buf));
    if(this->m_status_buf != NULL) Cbuffer_dispose(&(this->m_status_buf));
    if(this->m_value_buf != NULL) Cbuffer_dispose(&(this->m_value_buf));
    if(this->m_name_buf != NULL) Cbuffer_dispose(&(this->m_name_buf));
    free(this);
    *parser_p = NULL;

}
int Parser_append_bytes(http_parser_r this, void *buffer, unsigned length)
{
    // @TODO - need to handle error
    CHECK_TAG(HTTP_PARSER_TAG, this)
    llhttp_errno_t errno  = llhttp_execute(this->m_llhttp_ptr, (const char*)buffer, (int)length);
    size_t nparsed = (unsigned long)llhttp_get_error_pos(this->m_llhttp_ptr) - (unsigned long)buffer;
    return (int)nparsed;
}
//void Parser_begin(http_parser_r this, MessageRef message_ptr)
//{
//    http_parser_initialize(this);
//    this->m_current_message_ptr = message_ptr;
//}

llhttp_errno_t http_parser_consume(http_parser_r parser, const void* buffer, int length)
{
    CHECK_TAG(HTTP_PARSER_TAG, parser)
    char* b = (char*) buffer;
    int need_eof = llhttp_message_needs_eof(parser->m_llhttp_ptr);
    llhttp_errno_t x = http_parser_get_errno(parser);
    llhttp_errno_t errno = HPE_OK;
    if (length == 0) {
        errno = llhttp_finish(parser->m_llhttp_ptr);
    } else {
        errno = llhttp_execute(parser->m_llhttp_ptr, b, length);
//        int need_eof = llhttp_message_needs_eof(this->m_llhttp_ptr);
//        if(need_eof) {
//            return llhttp_finish(this->m_llhttp_ptr);
//        }
    }
    return errno;
}
llhttp_errno_t http_parser_get_errno(http_parser_t* this)
{
    CHECK_TAG(HTTP_PARSER_TAG, this)

    llhttp_errno_t x = llhttp_get_errno(this->m_llhttp_ptr);
    return x;
}
const void* http_parser_last_byte_parsed(http_parser_t* this)
{
    CHECK_TAG(HTTP_PARSER_TAG, this)
    const void* x = llhttp_get_error_pos(this->m_llhttp_ptr);
    return x;
}
http_parser_error_t http_parser_get_error(http_parser_r parser)
{
    CHECK_TAG(HTTP_PARSER_TAG, parser)
    llhttp_errno_t x = llhttp_get_errno(parser->m_llhttp_ptr);
    char* n = (char*)llhttp_errno_name(x);
    char* d = (char*)llhttp_errno_name(x);
    http_parser_error_t erst;
    erst.m_err_number = x;
    erst.m_name = n;
    erst.m_description = d;
    return erst;

}

void http_parser_initialize(http_parser_t* this)
{
    CHECK_TAG(HTTP_PARSER_TAG, this)

    this->m_header_state = kHEADER_STATE_NOTHING;
    this->m_started = false;
    this->current_message_ptr = Message_new();
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
    http_parser_r this =  (http_parser_r)(parser->data);
    CHECK_TAG(HTTP_PARSER_TAG, this)

    if(this->current_message_ptr != NULL) {
        Message_dispose(&(this->current_message_ptr));
        this->current_message_ptr = Message_new();
    } else {
        this->current_message_ptr = Message_new();
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
    http_parser_r this =  (http_parser_r)(parser->data);
    CHECK_TAG(HTTP_PARSER_TAG, this)
    Message_set_is_request(this->current_message_ptr, true);
    Cbuffer_append(this->m_url_buf, (char*)at, length); /*NEEDS ALLO TEST*/
    return 0;
}

static
int status_data_cb(llhttp_t* parser, const char* at, size_t length)
{
    http_parser_r this =  (http_parser_r)(parser->data);
    CHECK_TAG(HTTP_PARSER_TAG, this)
    Message_set_is_request(this->current_message_ptr, false);
    Message_set_status(this->current_message_ptr, this->m_llhttp_ptr->status_code);

    Cbuffer_append(this->m_status_buf, (char*)at, length);  /*NEEDS ALLO TEST*/
    Message_set_reason_cbuffer(this->current_message_ptr, this->m_status_buf);  /*NEEDS ALLO TEST*/
    return 0;
}
static
int method_data_cb(llhttp_t* parser, const char* at, size_t length)
{
    return 0;
    http_parser_r this =  (http_parser_r)(parser->data);
    Message_set_is_request(this->current_message_ptr, false);
    Message_set_status(this->current_message_ptr, this->m_llhttp_ptr->status_code);

    Cbuffer_append(this->m_status_buf, (char*)at, length);  /*NEEDS ALLO TEST*/
    Message_set_reason_cbuffer(this->current_message_ptr, this->m_status_buf);  /*NEEDS ALLO TEST*/
    return 0;
}
static
int version_data_cb(llhttp_t* parser, const char* at, size_t length)
{
    return 0;
    http_parser_r this =  (http_parser_r)(parser->data);
    Message_set_is_request(this->current_message_ptr, false);
    Message_set_status(this->current_message_ptr, this->m_llhttp_ptr->status_code);

    Cbuffer_append(this->m_status_buf, (char*)at, length);  /*NEEDS ALLO TEST*/
    Message_set_reason_cbuffer(this->current_message_ptr, this->m_status_buf);  /*NEEDS ALLO TEST*/
    return 0;
}

static
int header_field_data_cb(llhttp_t* parser, const char* at, size_t length)
{
    http_parser_r this =  (http_parser_r)(parser->data);
    CHECK_TAG(HTTP_PARSER_TAG, this)
    int state = this->m_header_state;
    if( (state == 0) || (state == kHEADER_STATE_NOTHING) || (state == kHEADER_STATE_VALUE)) {
        if(Cbuffer_size(this->m_name_buf) != 0) {
            Message_add_header_cbuf(this->current_message_ptr, this->m_name_buf, this->m_value_buf);
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
    http_parser_r this =  (http_parser_r)(parser->data);
    CHECK_TAG(HTTP_PARSER_TAG, this)
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
    http_parser_r this =  (http_parser_r)(parser->data);
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
    http_parser_r this =  (http_parser_r)(parser->data);
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
    http_parser_r this =  (http_parser_r)(parser->data);
    CHECK_TAG(HTTP_PARSER_TAG, this)
    if( Cbuffer_size(this->m_name_buf) != 0 ) {
        Message_add_header_cbuf(this->current_message_ptr, this->m_name_buf, this->m_value_buf);
        Cbuffer_clear(this->m_name_buf);
        Cbuffer_clear(this->m_value_buf);
    }
    Message_set_version(this->current_message_ptr, parser->http_major, parser->http_minor );
    if( Cbuffer_size(this->m_url_buf)  == 0 ) {
    } else {
        Message_set_method(this->current_message_ptr, (llhttp_method_t)parser->method);
        Message_set_target_cbuffer(this->current_message_ptr, this->m_url_buf);  /*NEEDS ALLO TEST*/
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
    http_parser_r this =  (http_parser_r)(parser->data);
    CHECK_TAG(HTTP_PARSER_TAG, this)
    BufferChainRef chain_ptr = Message_get_body(this->current_message_ptr);
    if (chain_ptr == NULL) {
        chain_ptr = BufferChain_new();  /*NEEDS ALLO TEST*/
        Message_set_body(this->current_message_ptr, chain_ptr);
    }
    BufferChain_append(chain_ptr, (void*)at, length); /*NEEDS ALLO TEST*/
    return 0;
}

static
int message_complete_cb(llhttp_t* parser)
{
    http_parser_r this =  (http_parser_r)(parser->data);
    CHECK_TAG(HTTP_PARSER_TAG, this)
    MessageRef tmp = this->current_message_ptr;
    this->current_message_ptr = Message_new();
    llhttp_errno_t retval = this->on_message_handler(this, tmp);
    return (int)retval;
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
    http_parser_r this =  (http_parser_r)(parser->data);
    return 0;
}
static
int chunk_complete_cb(llhttp_t* parser)
{
    http_parser_r p =  (http_parser_r)(parser->data);
    return 0;
}
static
int on_reset_cb(llhttp_t* parser)
{
    http_parser_r p =  (http_parser_r)(parser->data);
    return 0;
}
