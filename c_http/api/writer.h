#ifndef c_http_writer_h
#define c_http_writer_h
#include <c_http/api/message.h>
/**
 * @addtogroup group_writer Writer
 * @brief A module that implements a synchronous writer of http/1.1 messages
 * @{
 */


typedef struct Writer_s Writer, *WriterRef;

void Writer_init(WriterRef this, int sock);
WriterRef Writer_new(int sock);
void Writer_destroy(WriterRef this);
void Writer_dispose(WriterRef* this_ptr);

void Writer_write(WriterRef wrtr, MessageRef msg_ref);
//void Writer_start(WriterRef this, HttpStatus status, HdrListRef headers);
void Writer_write_chunk(WriterRef this, void* buffer, int len);
int Writer_sock_fd(WriterRef this);

/** @}*/
#endif