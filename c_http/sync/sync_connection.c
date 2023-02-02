#define _GNU_SOURCE
#include <c_http/sync/sync.h>
#include <c_http/sync/sync_internal.h>
#include <c_http/common/alloc.h>
#include <c_http/common/utils.h>
#include <c_http/common/iobuffer.h>
#include <c_http/http_parser/ll_parser.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#define sync_connection_TAG "SYNCCONN"
#include <c_http/check_tag.h>

//struct sync_connection_s
//{
//    DECLARE_TAG;
//    http_parser_t*              m_parser;
//    IOBufferRef                 m_iobuffer;
//    int                         socketfd;
//    size_t                      read_buffer_size;
//    SyncConnectionMessageHandler    handler;
//    void*                       handler_context;
//    int                 m_io_errno;
//    int                 m_http_errno;
//    char*               m_http_err_name;
//    char*               m_http_err_description;
//
//};

static void parser_on_message_handler(http_parser_t* context, MessageRef input_message_ref)
{
    sync_connection_t* connptr = context->handler_context;
    connptr->handler(input_message_ref, connptr);
}

void sync_connection_init(sync_connection_t* this, int socketfd, size_t read_buffer_size, SyncConnectionMessageHandler handler, void* handler_context)
{
    ASSERT_NOT_NULL(this);
    SET_TAG(sync_connection_TAG, this)
    this->m_parser = http_parser_new(&parser_on_message_handler, this);
    this->socketfd = socketfd;
//    this->m_rdsocket = rdsock;
    this->m_iobuffer = IOBuffer_new();
    this->read_buffer_size = read_buffer_size;
    this->handler = handler;
    this->handler_context = handler_context;
}

sync_connection_t* sync_connection_new(int socketfd, size_t read_buffer_size, SyncConnectionMessageHandler handler, void* handler_context)
{
    sync_connection_t* rdr = malloc(sizeof(sync_connection_t));
    if(rdr == NULL)
        return NULL;
    sync_connection_init(rdr, socketfd, read_buffer_size, handler, handler_context);
    return rdr;
}

void sync_connection_destroy(sync_connection_t* this)
{
    CHECK_TAG(sync_connection_TAG, this)
    IOBuffer_dispose(&(this->m_iobuffer));

}
void sync_connection_dispose(sync_connection_t** this_ptr)
{
    sync_connection_t* this = *this_ptr;
    CHECK_TAG(sync_connection_TAG, this)
    sync_connection_destroy(this);
    eg_free((void*)this);
    *this_ptr = NULL;
}
int sync_connection_read(sync_connection_t* this)
{
    CHECK_TAG(sync_connection_TAG, this)
    llhttp_errno_t rc;
    http_parser_t* pref = this->m_parser;
    while(1) {
        char buffer[1000];
        char* b = (char*) &buffer;
        int length = 900;
        size_t nread = read(this->socketfd, (void*)b, length);
        if(nread > 0) {
            rc = http_parser_consume(pref, (void *) buffer, nread);
            if(rc != HPE_OK) {
                return rc;
            }
        } else if(nread == 0) {
            rc = http_parser_consume(pref, NULL, 0);
            return rc;
        } else {
            return HPE_USER;
            break;
        }
    }
}
