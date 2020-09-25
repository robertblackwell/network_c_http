#ifndef c_eg_writer_h
#define c_eg_writer_h
#include <c_eg/message.h>
#include <c_eg/socket_functions.h>

struct Wrtr_s {
    socket_handle_t socket;
};

typedef struct Wrtr_s Wrtr, *WrtrRef;

void Wrtr_init(WrtrRef this, socket_handle_t socket);
WrtrRef Wrtr_new(socket_handle_t socket);
WrtrRef Wrtr_new(socket_handle_t socket);
void Wrtr_destroy(WrtrRef this);
void Wrtr_free(WrtrRef* this_ptr);

void Wrtr_write(WrtrRef wrtr, MessageRef msg_ref);
void Wrtr_start(WrtrRef this, HttpStatus status, HdrListRef headers);
void Wrtr_write_chunk(WrtrRef this, void* buffer, int len);

#endif