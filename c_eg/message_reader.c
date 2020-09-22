#define _GNU_SOURCE

#include <c_eg/message_reader.h>
#include <c_eg/alloc.h>
#include <c_eg/utils.h>
#include <c_eg/buffer/iobuffer.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

MessageReaderRef MessageReader_new(ParserRef parser, int socket)
{
    MessageReaderRef rdr = eg_alloc(sizeof(MessageReader));
    if(rdr == NULL)
        return NULL;
    MessageReader_init(rdr, parser, socket);
    return rdr;
}
void MessageReader_init(MessageReaderRef  this, ParserRef parser, int socket)
{
    ASSERT_NOT_NULL(this);
    this->m_parser = parser;
    this->m_socket = socket;
    this->m_iobuffer = IOBuffer_new();
}

void MessageReader_destroy(MessageReaderRef this)
{
    IOBuffer_free(&(this->m_iobuffer));

}
void MessageReader_free(MessageReaderRef* this_ptr)
{
    MessageReaderRef this = *this_ptr;
    MessageReader_destroy(this);
    eg_free((void*)this);
    *this_ptr = NULL;
}
MessageRef MessageReader_read(MessageReaderRef this)
{
    IOBufferRef ctx = this->m_iobuffer;
    MessageRef message_ptr = Message_new();
    Parser_begin(this->m_parser, message_ptr);
    int bytes_read;
    for(;;) {
        if(IOBuffer_data_len(ctx) == 0 ) {
            IOBuffer_reset(ctx);
            bytes_read = read(this->m_socket, IOBuffer_space(ctx), IOBuffer_space_len(ctx));
            // bytes_read == 0 means other end closed the socket
            // bytes_read < 0 means io error which includes I closed the socket on the last iteration of the read message loop
            // in either case return NULL unless the other end is signalling end of message with a close (non HTTP spec compliant)
            if(bytes_read <= 0) {
                if(this->m_parser->m_started && (!this->m_parser->m_message_done)) {
                    bytes_read = 0;
                } else {
                    Message_free(&(message_ptr));
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
    return message_ptr;
}
