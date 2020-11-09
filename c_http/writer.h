#ifndef c_http_writer_h
#define c_http_writer_h
#include <c_http/message.h>
#include <c_http/socket_functions.h>

struct Writer_s {
    int m_sock;
};

typedef struct Writer_s Writer, *WriterRef;

void Writer_init(WriterRef this, int sock);
WriterRef Writer_new(int sock);
WriterRef Writer_new(int sock);
void Writer_destroy(WriterRef this);
void Writer_free(WriterRef* this_ptr);

void Writer_write(WriterRef wrtr, MessageRef msg_ref);
void Writer_start(WriterRef this, HttpStatus status, HdrListRef headers);
void Writer_write_chunk(WriterRef this, void* buffer, int len);

#endif