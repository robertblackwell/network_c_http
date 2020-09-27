#ifndef c_eg_writer_h
#define c_eg_writer_h
#include <c_eg/message.h>
#include <c_eg/socket_functions.h>

struct Wrtr_s {
    int m_sock;
};

typedef struct Wrtr_s Wrtr;

void Wrtr_init(Wrtr* this, int sock);
Wrtr* Wrtr_new(int sock);
Wrtr* Wrtr_new(int sock);
void Wrtr_destroy(Wrtr* this);
void Wrtr_free(Wrtr** this_ptr);

void Wrtr_write(Wrtr* wrtr, Message* msg_ref);
void Wrtr_start(Wrtr* this, HttpStatus status, HdrList* headers);
void Wrtr_write_chunk(Wrtr* this, void* buffer, int len);

#endif