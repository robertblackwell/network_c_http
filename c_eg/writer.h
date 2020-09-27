#ifndef c_eg_writer_h
#define c_eg_writer_h
#include <c_eg/message.h>
#include <c_eg/socket_functions.h>

struct Writer_s {
    int m_sock;
};

typedef struct Writer_s Writer;

void Writer_init(Writer* this, int sock);
Writer* Writer_new(int sock);
Writer* Writer_new(int sock);
void Writer_destroy(Writer* this);
void Writer_free(Writer** this_ptr);

void Writer_write(Writer* wrtr, Message* msg_ref);
void Writer_start(Writer* this, HttpStatus status, HdrList* headers);
void Writer_write_chunk(Writer* this, void* buffer, int len);

#endif