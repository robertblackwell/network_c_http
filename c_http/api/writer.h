#ifndef c_http_writer_h
#define c_http_writer_h
#include <c_http/api/message.h>
/**
 * @addtogroup group_writer Writer
 * @brief A module that implements a synchronous writer of http/1.1 messages
 * @{
 */

#define TYPE Writer
#define Writer_TAG "WRITER"
#include <c_http/check_tag.h>
#undef TYPE
#define WRITER_DECLARE_TAG DECLARE_TAG(Writer)
#define WRITER_CHECK_TAG(p) CHECK_TAG(Writer, p)
#define WRITER_SET_TAG(p) SET_TAG(Writer, p)

typedef struct Writer_s Writer, *WriterRef;

void Writer_init(WriterRef this, int sock);
WriterRef Writer_new(int sock);
void Writer_destroy(WriterRef this);
void Writer_free(WriterRef* this_ptr);

void Writer_write(WriterRef wrtr, MessageRef msg_ref);
//void Writer_start(WriterRef this, HttpStatus status, HdrListRef headers);
void Writer_write_chunk(WriterRef this, void* buffer, int len);
int Writer_sock_fd(WriterRef this);

/** @}*/
#endif