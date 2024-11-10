
#include <http_in_c/common/rdsocket.h>
#include <http_in_c/demo_protocol/demo_sync_reader.h>
#include <http_in_c/common/alloc.h>
#include <http_in_c/common/utils.h>
#include <http_in_c/common/iobuffer.h>
#include <http_in_c/demo_protocol/demo_parser.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#define DemoSyncReader_TAG "DSYRDR"
#include <rbl/check_tag.h>


struct DemoSyncReader_s
{
    RBL_DECLARE_TAG;
    DemoParserRef       m_parser;
    IOBufferRef         m_iobuffer;
    int                 m_socket_fd;
    ListRef             input_message_list;
    ListRef             output_message_list;
    int                 m_io_errno;
    int                 m_http_errno;
    char*               m_http_err_name;
    char*               m_http_err_description;
    RBL_DECLARE_END_TAG;
};
void on_new_message(void* arg, DemoMessageRef msg)
{
    printf("got a message");
    DemoSyncReader* rdr = arg;
    List_add_back(rdr->input_message_list, msg);
}
void demosync_reader_init(DemoSyncReaderRef  this, int socket_fd)
{
    ASSERT_NOT_NULL(this);
    RBL_SET_TAG(DemoSyncReader_TAG, this)
    RBL_SET_END_TAG(DemoSyncReader_TAG, this)
    this->m_parser = DemoParser_new(on_new_message, this);
    this->m_socket_fd = socket_fd;
    this->m_iobuffer = IOBuffer_new();
    this->input_message_list = List_new(NULL);
    this->output_message_list = List_new(NULL);
}

DemoSyncReaderRef demosync_reader_private_new(int socket_fd)
{
    DemoSyncReaderRef rdr = eg_alloc(sizeof(DemoSyncReader));
    if(rdr == NULL)
        return NULL;
    demosync_reader_init(rdr, socket_fd);
    return rdr;
}

DemoSyncReaderRef demosync_reader_new(int socket_fd)
{
    return demosync_reader_private_new(socket_fd);
}

void demosync_reader_destroy(DemoSyncReaderRef this)
{
    RBL_CHECK_TAG(DemoSyncReader_TAG,this)
    RBL_CHECK_END_TAG(DemoSyncReader_TAG,this)
    IOBuffer_dispose(&(this->m_iobuffer));

}
void demosync_reader_dispose(DemoSyncReaderRef* this_ptr)
{
    DemoSyncReaderRef this = *this_ptr;
    RBL_CHECK_TAG(DemoSyncReader_TAG,this)
    RBL_CHECK_END_TAG(DemoSyncReader_TAG,this)
    demosync_reader_destroy(this);
    eg_free((void*)this);
    *this_ptr = NULL;
}
int demosync_reader_read(DemoSyncReaderRef this, DemoMessageRef* msgref_ptr)
{
    RBL_CHECK_TAG(DemoSyncReader_TAG,this)
    RBL_CHECK_END_TAG(DemoSyncReader_TAG,this)
    IOBufferRef iobuf = this->m_iobuffer;
    DemoMessageRef message_ptr = demo_message_new();
    long bytes_read;
    while(1) {
        IOBuffer_reset(iobuf);
        void* buf = IOBuffer_space(iobuf);
        int len = IOBuffer_space_len(iobuf);
        bytes_read = read(this->m_socket_fd, buf, len);
        if(bytes_read >  0) {
            IOBuffer_commit(iobuf, (int)bytes_read);
            printf("demosync_reader response raw: %s \n", IOBuffer_cstr(iobuf));
            DemoParser_consume(this->m_parser, iobuf);
            while(List_size(this->input_message_list) > 0) {
                DemoMessageRef msg_ref = List_remove_first(this->input_message_list);
            }
            return;
        } else if (bytes_read == 0) {
        } else {
        }
    }
}
