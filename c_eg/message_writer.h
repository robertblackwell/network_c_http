#ifndef c_eg_message_writer_h
#define c_eg_message_writer_h
#include <c_eg/message.h>
#include <c_eg/socket_functions.h>

struct MessageWriter_s {
    socket_handle_t socket;
};

typedef struct MessageWriter_s MessageWriter, *MessageWriterRef;

void MessageWriter_init(MessageWriterRef this, socket_handle_t socket);
MessageWriterRef MessageWriter_new(socket_handle_t socket);
MessageWriterRef MessageWriter_new(socket_handle_t socket);
void MessageWriter_destroy(MessageWriterRef this);
void MessageWriter_free(MessageWriterRef* this_ptr);

void MessageWriter_write(MessageWriterRef wrtr, MessageRef msg_ref);
void MessageWriter_start(MessageWriterRef this, HttpStatus status, HDRListRef headers);
void MessageWriter_write_chunk(MessageWriterRef this, void* buffer, int len);

#endif