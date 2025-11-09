

#include <rbl/macros.h>
#include <src/common/socket_functions.h>
#include <src/common/alloc.h>
#include <src/common/utils.h>
#include <src/common/iobuffer.h>
#include <src/http/http_message_parser.h>
#include <rbl/logger.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#define sync_connection_TAG "SYNCCONN"
#include <rbl/check_tag.h>
#include "sync_client.h"

static void parser_on_message_handler(void* void_parser_ptr, HttpMessageRef input_message_ref, int error)
{
    HttpMessageParserRef parser_ref = void_parser_ptr;
    sync_connection_t* connptr = parser_ref->on_message_handler_context;
    RBL_ASSERT((input_message_ref != NULL), "obvious");
    List_add_back(connptr->input_list, input_message_ref);
    connptr->reader_status = 1;
}
static void message_dealloc(void** p)
{
    void* mref = *p;
    http_message_free(*p);
    *p = NULL;
}
void sync_connection_init(sync_connection_t* this, int socketfd, size_t read_buffer_size) //, SyncConnectionServerMessageHandler handler, sync_worker_r worker_ref)
{
    ASSERT_NOT_NULL(this);
    RBL_SET_TAG(SYNC_CONNECTION_TAG, this)
    RBL_LOG_FMT("sync_connection_init socketfd: %d", socketfd);
    this->m_parser = http_message_parser_new(&parser_on_message_handler, this);
    this->socketfd = socketfd;
//    this->m_rdsocket = rdsock;
    this->m_iobuffer = IOBuffer_new();
    this->input_list = List_new();
    this->reader_status = 0;
    this->read_buffer_size = read_buffer_size;
//    this->callback.server_cb.server_handler = NULL;
//    this->callback.server_cb.worker_ptr = NULL;
}

sync_connection_t* sync_connection_new(int socketfd, size_t read_buffer_size) //, SyncConnectionServerMessageHandler handler, sync_worker_r worker_ref)
{
    sync_connection_t* rdr = malloc(sizeof(sync_connection_t));
    if(rdr == NULL)
        return NULL;
    sync_connection_init(rdr, socketfd, read_buffer_size); //, handler, worker_ref);
    return rdr;
}
void sync_connection_destroy(sync_connection_t* this)
{
    RBL_CHECK_TAG(SYNC_CONNECTION_TAG, this)
    IOBuffer_free(this->m_iobuffer);
    this->m_iobuffer = NULL;
    RBL_INVALIDATE_TAG(this)
    // RBL_INVALIDATE_STRUCT(this, sync_connection_t)
}
void sync_connection_dispose(sync_connection_t** this_ptr)
{
    sync_connection_t* this = *this_ptr;
    RBL_CHECK_TAG(SYNC_CONNECTION_TAG, this)
    sync_connection_destroy(this);
    eg_free((void*)this);
    *this_ptr = NULL;
}
void sync_connection_close(sync_connection_t* this)
{
    RBL_CHECK_TAG(SYNC_CONNECTION_TAG, this)
    RBL_LOG_FMT("sync_connection_close %p socketfd: %d", this, this->socketfd);
    close(this->socketfd);
    this->socketfd = -1;
}
