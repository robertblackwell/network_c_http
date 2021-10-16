#define _GNU_SOURCE
#include <c_http/common/http_parser/rdsocket.h>
#include <c_http/sync/sync_reader.h>
#include <c_http/common/alloc.h>
#include <c_http/common/utils.h>
#include <c_http/common/iobuffer.h>
#include <c_http/common/http_parser/ll_parser.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#define TYPE Reader
#define SyncReader_TAG "SYNCRDR"
#include <c_http/check_tag.h>
#undef TYPE
#define READER_DECLARE_TAG DECLARE_TAG(SyncReader)
#define READER_CHECK_TAG(p) CHECK_TAG(SyncReader, p)
#define READER_SET_TAG(p) SET_TAG(SyncReader, p)


struct SyncReader_s
{
    READER_DECLARE_TAG;
    ParserRef           m_parser;
    IOBufferRef         m_iobuffer;
    RdSocket            m_rdsocket;
    int                 m_io_errno;
    int                 m_http_errno;
    char*               m_http_err_name;
    char*               m_http_err_description;

};
void SyncReader_init(SyncReaderRef  this, RdSocket rdsock)
{
    ASSERT_NOT_NULL(this);
    READER_SET_TAG(this)
    this->m_parser = Parser_new();
//    this->m_socket = socket;
    this->m_rdsocket = rdsock;
    this->m_iobuffer = IOBuffer_new();
}

SyncReaderRef SyncReader_private_new(RdSocket rdsock)
{
    SyncReaderRef rdr = eg_alloc(sizeof(Reader));
    if(rdr == NULL)
        return NULL;
    SyncReader_init(rdr, rdsock);
    return rdr;
}

SyncReaderRef SyncReader_new(int rdsock_fd)
{
    return SyncReader_private_new(RealSocket(rdsock_fd));
//    SyncReaderRef rdr = eg_alloc(sizeof(Reader));
//    if(rdr == NULL)
//        return NULL;
//    SyncReader_init(rdr, rdsock);
//    return rdr;
}

void SyncReader_destroy(SyncReaderRef this)
{
    READER_CHECK_TAG(this)
    IOBuffer_dispose(&(this->m_iobuffer));

}
void SyncReader_dispose(SyncReaderRef* this_ptr)
{
    SyncReaderRef this = *this_ptr;
    READER_CHECK_TAG(this)
    SyncReader_destroy(this);
    eg_free((void*)this);
    *this_ptr = NULL;
}
int SyncReader_read(SyncReaderRef this, MessageRef* msgref_ptr)
{
    READER_CHECK_TAG(this)
    IOBufferRef iobuf = this->m_iobuffer;
    MessageRef message_ptr = Message_new();
    Parser_begin(this->m_parser, message_ptr);
    int bytes_read;
    for(;;) {
        // 
        // handle nothing left in iobuffer
        // only read more if iobuffer is empty
        // 
        if(IOBuffer_data_len(iobuf) == 0 ) {
            IOBuffer_reset(iobuf);
            void* buf = IOBuffer_space(iobuf); 
            int len = IOBuffer_space_len(iobuf);
            void* c = this->m_rdsocket.ctx;
            ReadFunc rf = (this->m_rdsocket.read_f);
            bytes_read = RdSocket_read(&(this->m_rdsocket), buf, len);
            if(bytes_read == 0) {
                if (! this->m_parser->m_started) {
                    // eof no message started - there will not be any more bytes to parse so cleanup and exit
                    // return no error 
                    Message_dispose(&(message_ptr));
                    *msgref_ptr = NULL;
                    return 0;
                }
                if (this->m_parser->m_started && this->m_parser->m_message_done) {
                    // shld not get here
                    assert(false);
                }
                if (this->m_parser->m_started && !this->m_parser->m_message_done) {
                    // get here if otherend is signlaling eom with eof
                    assert(true);
                }
            } else if (bytes_read > 0) {
                IOBuffer_commit(iobuf, bytes_read);
            } else {
                // have an io error
                int x = errno;
                printf("SyncReader_read io error errno: %d \n", x);
                Message_dispose(&(message_ptr));
                *msgref_ptr = NULL;
                return READER_IO_ERROR;
            }
        } else {
            bytes_read = IOBuffer_data_len(iobuf);
        }
        char* tmp = IOBuffer_data(iobuf);
        char* tmp2 = IOBuffer_memptr(iobuf);
        ParserReturnValue ret = Parser_consume(this->m_parser, (void*) IOBuffer_data(iobuf), IOBuffer_data_len(iobuf));
        int consumed = bytes_read - ret.bytes_remaining;
        IOBuffer_consume(iobuf, consumed);
        int tmp_remaining = IOBuffer_data_len(iobuf);
        switch(ret.return_code) {
            case ParserRC_error:
                ///
                /// got a parse error - need some way to signal the caller so can send reply of bad message
                ///
                Message_dispose(&message_ptr);
                *msgref_ptr = NULL;
                printf("SyncReader_read parser error \n");
                return READER_PARSE_ERROR;
                break;
            case ParserRC_end_of_data:
                break;
            case ParserRC_end_of_header:
                break;
            case ParserRC_end_of_message:
                *msgref_ptr = message_ptr;
                // return ok
                return READER_OK;
        }
    }
}
