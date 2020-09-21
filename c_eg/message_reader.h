#ifndef c_c_eg_message_reader_h
#define c_c_eg_message_reader_h
#include <c_eg/list.h>
#include <c_eg/message.h>
#include <c_eg/parser.h>
#include <c_eg/buffer/iobuffer.h>
#include <c_eg/socket_functions.h>

typedef struct MessageReader_s
{
    ParserRef           m_parser;
    int                 m_socket;
    IOBufferRef         m_iobuffer;
} MessageReader, *MessageReaderRef;

MessageReaderRef MessageReader_new(ParserRef parser, int socket);
void MessageReader_init(MessageReaderRef this, ParserRef parser, int socket);
void MessageReader_destroy(MessageReaderRef this);
void MessageReader_free(MessageReaderRef* this_ptr);
MessageRef MessageReader_read(MessageReaderRef this);

#endif