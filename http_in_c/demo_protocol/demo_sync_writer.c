#define _GNU_SOURCE
#include <http_in_c/demo_protocol/demo_sync_writer.h>
#include <http_in_c/common/alloc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <http_in_c/common/http_parser/ll_parser_types.h>
#include <http_in_c/common/iobuffer.h>

#define TYPE DemoSyncWriter
#define DemoSyncWriter_TAG "SYNCWRTR"
#include <http_in_c/check_tag.h>
#undef TYPE
#define DEMOSYNCWRITER_DECLARE_TAG DECLARE_TAG(DemoSyncWriter)
#define DEMOSYNCWRITER_CHECK_TAG(p) CHECK_TAG(DemoSyncWriter, p)
#define DEMOSYNCWRITER_SET_TAG(p) SET_TAG(DemoSyncWriter, p)


struct DemoSyncWriter_s {
    DEMOSYNCWRITER_DECLARE_TAG;
    int m_sock;
};

void demosync_writer_init(DemoSyncWriterRef this, int sock)
{
    DEMOSYNCWRITER_SET_TAG(this)
    this->m_sock = sock;
}
DemoSyncWriterRef demosync_writer_new(int socket)
{
    DemoSyncWriterRef mwref = eg_alloc(sizeof(DemoSyncWriter));
    if(mwref == NULL)
        return NULL;
    demosync_writer_init(mwref, socket);
    return mwref;
}
void demosync_writer_destroy(DemoSyncWriterRef this)
{
    DEMOSYNCWRITER_CHECK_TAG(this)
}
void demosync_writer_dispose(DemoSyncWriterRef* this_ptr)
{
    DemoSyncWriterRef this = *(this_ptr);
    DEMOSYNCWRITER_CHECK_TAG(this)
    demosync_writer_destroy(this);
    eg_free(*this_ptr);
    *this_ptr = NULL;
}
/**
 *
 * Write a buffer of given length. TODO Must handle partial writes
 *
 * \param this    WrtRef (holds the socket/fd to which the write should be made
 * \param buffer  void* Address of start of buffer
 * \param len     int length of data in buffer
 * \return        (TODO) return success, EOF-socket closed by other end, IO error
 */
void demosync_writer_write_chunk(DemoSyncWriterRef this, void* buffer, int len)
{
    DEMOSYNCWRITER_CHECK_TAG(this)
    char* c = (char*)buffer;
    void* tmp_buffer = buffer;
    int tmp_len = len;
    int my_errno;
    while(1) {
        int res = (int)write(this->m_sock, tmp_buffer, tmp_len);
        if(res == tmp_len) {
            return; // success
        } else if(res > 0) {
            // partial write
            tmp_buffer = tmp_buffer + res;
            tmp_len = tmp_len - res;
        } else if(res == 0) {
            // other end closed socket
            return; //eof
        } else if(res == -1) {
            my_errno = errno;
            return; //io_error
        }
    }
}

void demosync_writer_write(DemoSyncWriterRef this, DemoMessageRef msg_ref)
{
    IOBufferRef serialized = demo_message_serialize(msg_ref);
    demosync_writer_write_chunk(this, IOBuffer_data(serialized), IOBuffer_data_len(serialized));
}
/**
 * TODO - intended this function to write the entire body in one go and ensure a content-length header was included
 *
 * \param this
 */
void demosync_writer_end(DemoSyncWriterRef this)
{
    DEMOSYNCWRITER_CHECK_TAG(this)
}
int demosync_writer_sock_fd(DemoSyncWriterRef this)
{
    DEMOSYNCWRITER_CHECK_TAG(this)
    return this->m_sock;
}
