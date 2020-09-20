
#include <c_eg/parser.hpp>


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

ParserRef Parser_new()
{
    ParserRef this = eg_alloc(sizeof(Parser));
    if(this == NULL)
        return NULL;
    this->message_done = false;
    this->header_done = false;
    this->m_http_parser_ptr = NULL;
    this->m_http_parser_settings_ptr = NULL;
    this->header_state = kHEADER_STATE_NOTHING;
    return this;
}

void Parser_free(PraserRef* this_p)
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


ParserReturnValue Parser_consume(ParserRef this, const void* buf, size_t length, bool only_header)
{
    started = true;
    ReturnValue rv{.return_code = ReturnCode::end_of_data, .bytes_remaining = length};
    char* b = (char*) buf;
    size_t total_parsed = 0;
    while (total_parsed < length) {
        char* b_start_ptr = &(b[total_parsed]);
        int nparsed = this->appendBytes((void*) b_start_ptr, length - total_parsed);
        total_parsed = total_parsed + nparsed;
        // std::cout << "nparsed: " << nparsed  << "len: " << length - total_parsed << " content: " << buf <<  std::endl;
        rv.bytes_remaining = length - nparsed;
        if (this->is_error()) {
            rv.return_code = ReturnCode::error;
            auto x = get_error();
            return rv;
        } else if (this->message_done) {
            rv.return_code = ReturnCode::end_of_message;
            return rv;
        } else if (this->header_done) {
            rv.return_code = ReturnCode::end_of_header;
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
    ReturnValue rv{.return_code = ReturnCode::end_of_data, .bytes_remaining = 0};
    char* buffer = NULL;
    size_t nparsed;
    int someLength = 0;
    if( ! this->m_message_done ) {
        if (this->m_started) {
            nparsed = http_parser_execute(this->m_http_parser_ptr, this->m_http_parser_settings_ptr, buffer, someLength);
        }
        if (this->is_error()) {
            rv.return_code = ReturnCode::error;
            auto x = this->get_error();
            return rv;
        } else if (this->message_done) {
            rv.return_code = ReturnCode::end_of_message;
            return rv;
        } else if (this->header_done) {
            rv.return_code = ReturnCode::end_of_header;
        } else {
            printf("should not be here\n");
            return rv;
        }
        return rv;
    }
    return rv;
}
void Parser_append_eof(ParserRef pref)
{
    char* buffer = NULL;
    size_t nparsed;
    int someLength = 0;
    if( ! this->message_done )
    {
        nparsed = http_parser_execute(this->m_http_parser_ptr, this->m_http_parser_settings_ptr, buffer, someLength);
    }
    printf("back from parser nparsed: %d\n ", nparsed);
}
enum http_errno Parser_get_errno(ParserRef this)
{
    return (enum http_errno) this->m_http_parser_ptr->http_errno;
}
ParserError Parser_get_error(ParserRef this)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wall"
    enum http_errno x = (enum http_errno)this->m_http_parser_ptr->http_errno;
    char* n = (char*)http_errno_name(x);
    char* d = (char*)http_errno_description(x);
    ParserError erst;
    erst.err_number = x;
    erst.name = n;
    erst.description = d;
#pragma clang diagnostic pops
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
    this->m_current_message_sptr = NULL;
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
    ContigBuffer_clear(this->m_status_buf);
    ContigBuffer_clear(this->m_url_buf);
    ContigBuffer_clear(this->m_name_buf);
    ContigBuffer_clear(this->m_value_buf);
    m_http_parser_settings_ptr->on_message_begin = message_begin_cb;
    m_http_parser_settings_ptr->on_url = url_data_cb;
    m_http_parser_settings_ptr->on_status = status_data_cb;
    m_http_parser_settings_ptr->on_header_field = header_field_data_cb;
    m_http_parser_settings_ptr->on_header_value = header_value_data_cb;
    m_http_parser_settings_ptr->on_headers_complete = headers_complete_cb;
    m_http_parser_settings_ptr->on_body = body_data_cb;
    m_http_parser_settings_ptr->on_message_complete = message_complete_cb;
    m_http_parser_settings_ptr->on_chunk_header = chunk_header_cb;
    m_http_parser_settings_ptr->on_chunk_complete = chunk_complete_cb;
}


int message_begin_cb(http_parser* parser)
{
    ParserRef this =  (ParserRef)(parser->data);
    ContigBuffer_clear(this->m_status_buf);
    ContigBuffer_clear(this->m_url_buf);
    ContigBuffer_clear(this->m_name_buf);
    ContigBuffer_clear(this->m_value_buf);
    return 0;
}

int url_data_cb(http_parser* parser, const char* at, size_t length)
{
    ParserRef this =  (ParserRef)(parser->data);
    MessageRef message = Parser_current_message(this);
    Message_set_is_request(this->message, true);
    ContigBuffer_append(this->m_url_buf, (char*)at, length);
    return 0;
}

int status_data_cb(http_parser* parser, const char* at, size_t length)
{
    ParserRef this =  (ParserRef)(parser->data);
    MessageRef message = Parser_current_message(this);
    message->set_is_request(false);
    message->status_code(m_http_parser_ptr->status_code);
    status_stringbuf.append((char*)at, length);
    return 0;
}
int header_field_data_cb(http_parser* parser, const char* at, size_t length)
{
    ParserRef this =  (ParserRef)(parser->data);
    MessageRef message = Parser_current_message(this);
    int state = this->m_header_state;
    if( (state == 0) || (state == kHEADER_STATE_NOTHING) || (state == kHEADER_STATE_VALUE)) {
        if(ContigBuffer_size(this->m_name_buf) != 0) {
            current_message()->header(&name_stringbuf, &value_stringbuf);
            ContigBuffer_clear(this->m_name_buf);
            ContigBuffer_clear(this->m_value_buf);
        }
        ContigBuffer_append(this->m_name_buf, (char*)at, length);
    } else if( state == kHEADER_STATE_FIELD ) {
        ContigBuffer_append(this->m_name_buf, (char*)at, length);
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
        ContigBuffer_clear(this->m_value_buf);
        ContigBuffer_append(this->m_value_buf, (char*)at, length);
    } else if( state == kHEADER_STATE_VALUE) {
        ContigBuffer_append(this->m_value_buf, (char*)at, length);
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
    if( ContigBuffer_size(this->m_name_buf) != 0 ) {
        HDRList_add(Message_headers(message), this->m_name_buf, this->m_value_buf);
        ContigBuffer_clear(this->m_name_buf);
        ContigBuffer_clear(this->m_value_buf);
    }
    message->version( parser->http_major, parser->http_minor );
    if( ContigBuffer_size(this->m_url_buf)  == 0 ) {
    } else {
        Message_set_method(message, (enum http_method)parser->method);
        Message_set_target(message, this->m_url_buf);
    }
    if( ContigBuffer_size(this->m_status_buf) == 0 ) {
    } else {
        Message_set_reason(message, this->status_buf);
    }
    this->m_header_done = true;
    return 0;
}
int body_data_cb(http_parser* parser, const char* at, size_t length)
{
    ParserRef p =  (ParserRef)(parser->data);
    MessageRef message = Parser_current_message(this);
    BufferChainRef chain_ptr = Message_get_chain_buf(message);
    if (chain_ptr == NULL) {
        chain_ptr = ContigBuffer_new();
        Message_set_body_chain(message, chain_ptr);
    }
    BufferChain_append(this->m_chain_ptr, (void*)at, length);
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
    ParserRef p =  (ParserRef)(parser->data);
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
