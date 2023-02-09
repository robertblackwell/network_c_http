#define _GNU_SOURCE
#include <http_in_c/saved/rdsocket.h>
#include <http_in_c/saved/sync_reader.h>
#include <http_in_c/common/alloc.h>
#include <http_in_c/common/utils.h>
#include <http_in_c/common/iobuffer.h>
#include <http_in_c/http/parser.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#define SyncReader_TAG "SYNCRDR"
#include <http_in_c/check_tag.h>


struct SyncReader_s
{
    DECLARE_TAG;
    http_parser_t*              m_parser;
    IOBufferRef                 m_iobuffer;
    int                         m_socket;
    RdSocket*                   m_rdsocket;
    OnMessageCompleteHandler    handler;
    void*                       handler_context;
    int                 m_io_errno;
    int                 m_http_errno;
    char*               m_http_err_name;
    char*               m_http_err_description;

};
void SyncReader_init(SyncReaderRef  this, RdSocket rdsock, OnMessageCompleteHandler handler, void* handler_context)
{
    ASSERT_NOT_NULL(this);
    SET_TAG(SyncReader_TAG, this)
    this->m_parser = http_parser_new(handler, handler_context);
//    this->m_socket = socket;
    this->m_rdsocket = rdsock;
    this->m_iobuffer = IOBuffer_new();
    this->handler = handler;
    this->handler_context = handler_context;
}

SyncReaderRef SyncReader_private_new(RdSocket rdsock, OnMessageCompleteHandler handler, void* handler_context)
{
    SyncReaderRef rdr = eg_alloc(sizeof(Reader));
    if(rdr == NULL)
        return NULL;
    SyncReader_init(rdr, rdsock, handler, handler_context);
    return rdr;
}

SyncReaderRef SyncReader_new(int rdsock_fd, OnMessageCompleteHandler handler, void* handler_context)
{
    return SyncReader_private_new(RealSocket(rdsock_fd), handler, handler_context);
}

void SyncReader_destroy(SyncReaderRef this)
{
    CHECK_TAG(SyncReader_TAG, this)
    IOBuffer_dispose(&(this->m_iobuffer));

}
void SyncReader_dispose(SyncReaderRef* this_ptr)
{
    SyncReaderRef this = *this_ptr;
    CHECK_TAG(SyncReader_TAG, this)
    SyncReader_destroy(this);
    eg_free((void*)this);
    *this_ptr = NULL;
}
int SyncReader_read(SyncReaderRef this, MessageRef* msgref_ptr)
{
    CHECK_TAG(SyncReader_TAG, this)
    llhttp_errno_t rc;
    http_parser_t* pref = this->m_parser;
    while(1) {
        char buffer[1000];
        char* b = (char*) &buffer;
        int length = 900;
        int status = RdSocket_read(this->m_rdsocket, (void*)b, length);
        if(status > 0) {
            rc = http_parser_consume(pref, (void *) buffer, status);
            if(rc != HPE_OK) {
                return rc;
            }
        } else if(status == 0) {
            rc = http_parser_consume(pref, NULL, 0);
            return rc;
        } else {
            return HPE_USER;
            break;
        }
    }
}
