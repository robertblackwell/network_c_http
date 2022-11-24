#define _GNU_SOURCE
#include <c_http/common/http_parser/rdsocket.h>
#include <c_http/demo_protocol/demo_sync_reader.h>
#include <c_http/common/alloc.h>
#include <c_http/common/utils.h>
#include <c_http/common/iobuffer.h>
#include <c_http/demo_protocol/demo_parser.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#define TYPE Reader
#define DemoSyncReader_TAG "DSYRDR"
#include <c_http/check_tag.h>
#undef TYPE
#define DEMOSYNCREADER_DECLARE_TAG DECLARE_TAG(DemoSyncReader)
#define DEMOSYNCREADER_CHECK_TAG(p) CHECK_TAG(DemoSyncReader, p)
#define DEMOSYNCREADER_SET_TAG(p) SET_TAG(DemoSyncReader, p)


struct DemoSyncReader_s
{
    DEMOSYNCREADER_DECLARE_TAG;
    DemoParserRef           m_parser;
    IOBufferRef         m_iobuffer;
    RdSocket            m_rdsocket;
    int                 m_io_errno;
    int                 m_http_errno;
    char*               m_http_err_name;
    char*               m_http_err_description;

};
void demosync_reader_init(DemoSyncReaderRef  this, RdSocket rdsock)
{
    ASSERT_NOT_NULL(this);
    DEMOSYNCREADER_SET_TAG(this)
    this->m_parser = DemoParser_new();
//    this->m_socket = socket;
    this->m_rdsocket = rdsock;
    this->m_iobuffer = IOBuffer_new();
}

DemoSyncReaderRef demosync_reader_private_new(RdSocket rdsock)
{
    DemoSyncReaderRef rdr = eg_alloc(sizeof(DemoSyncReader));
    if(rdr == NULL)
        return NULL;
    demosync_reader_init(rdr, rdsock);
    return rdr;
}

DemoSyncReaderRef demosync_reader_new(int rdsock_fd)
{
    return demosync_reader_private_new(RealSocket(rdsock_fd));
//    DemoSyncReaderRef rdr = eg_alloc(sizeof(Reader));
//    if(rdr == NULL)
//        return NULL;
//    demosync_reader_init(rdr, rdsock);
//    return rdr;
}

void demosync_reader_destroy(DemoSyncReaderRef this)
{
    DEMOSYNCREADER_CHECK_TAG(this)
    IOBuffer_dispose(&(this->m_iobuffer));

}
void demosync_reader_dispose(DemoSyncReaderRef* this_ptr)
{
    DemoSyncReaderRef this = *this_ptr;
    DEMOSYNCREADER_CHECK_TAG(this)
    demosync_reader_destroy(this);
    eg_free((void*)this);
    *this_ptr = NULL;
}
int demosync_reader_read(DemoSyncReaderRef this, DemoMessageRef* msgref_ptr)
{
    DEMOSYNCREADER_CHECK_TAG(this)
    IOBufferRef iobuf = this->m_iobuffer;
    DemoMessageRef message_ptr = demo_message_new();
    DemoParser_begin(this->m_parser, message_ptr);
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
                    demo_message_dispose(&(message_ptr));
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
                printf("demosync_reader response raw: %s \n", IOBuffer_cstr(iobuf));
            } else {
                // have an io error
                int x = errno;
                printf("demosync_reader_read io error errno: %d \n", x);
                demo_message_dispose(&(message_ptr));
                *msgref_ptr = NULL;
                return READER_IO_ERROR;
            }
        } else {
            bytes_read = IOBuffer_data_len(iobuf);
        }
        char* tmp = IOBuffer_data(iobuf);
        char* tmp2 = IOBuffer_memptr(iobuf);
        DemoParserPrivateReturnValue_s ret = DemoParser_consume(this->m_parser, (void*) IOBuffer_data(iobuf), IOBuffer_data_len(iobuf));
        int consumed = ret.bytes_consumed;
        IOBuffer_consume(iobuf, consumed);
        int tmp_remaining = IOBuffer_data_len(iobuf);
#if 0
        switch(ret.return_code) {
            case DemoParserRC_invalid_opcode:
                ///
                /// got a parse error - need some way to signal the caller so can send reply of bad message
                ///
                demo_message_dispose(&message_ptr);
                *msgref_ptr = NULL;
                printf("demosync_reader_read parser error \n");
                return READER_PARSE_ERROR;
                break;
            case DemoParserRC_expected_stx:
            case DemoParserRC_expected_ascii:
                break;
            case DemoParserRC_end_of_message:
                *msgref_ptr = message_ptr;
                // return ok
                return READER_OK;
        }
#endif
    }
}
