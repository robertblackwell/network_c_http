#define _GNU_SOURCE

#include <c_http/xr/xr_reader.h>
#include <c_http/alloc.h>
#include <c_http/utils.h>
#include <c_http/buffer/iobuffer.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>

//XrReaderRef XrReader_new(ParserRef parser, RdSocket rdsock)
//{
//    XrReaderRef rdr = eg_alloc(sizeof(XrReader));
//    if(rdr == NULL)
//        return NULL;
//    XrReader_init(rdr, parser, rdsock);
//    return rdr;
//}
void XrReader_init(XrReaderRef  this, ParserRef parser, int socket)
{
    ASSERT_NOT_NULL(this);
    this->m_parser = parser;
    this->m_socket = socket;
    this->m_iobuffer = IOBuffer_new();
}

void XrReader_destroy(XrReaderRef this)
{
    IOBuffer_free(&(this->m_iobuffer));

}
void XrReader_free(XrReaderRef* this_ptr)
{
    XrReaderRef this = *this_ptr;
    XrReader_destroy(this);
    eg_free((void*)this);
    *this_ptr = NULL;
}
static void free_message(XrReaderRef this)
{
    if(this->m_message_ref != NULL) {
        Message_free(&(this->m_message_ref));
    }
}
int XrReader_read(XrReaderRef this)
{
    IOBufferRef iobuf = this->m_iobuffer;
    if(this->m_message_ref == NULL) {
        this->m_message_ref = Message_new();
    }
    Parser_begin(this->m_parser, this->m_message_ref);
    int bytes_read;
    int errno_saved;
    for(;;) {
        // 
        // handle nothing left in iobuffer
        // only read more if iobuffer is empty
        // 
        if(IOBuffer_data_len(iobuf) == 0 ) {
            IOBuffer_reset(iobuf);
            void* buf = IOBuffer_space(iobuf); 
            int len = IOBuffer_space_len(iobuf);
            bytes_read = read(this->m_socket, buf, len);
            errno_saved = errno;
            if(bytes_read == 0) {
                if (! this->m_parser->m_started) {
                    // eof no message started - there will not be any more bytes to parse so cleanup and return eof
                    free_message(this);
                    return XR_READER_EOF;
                }
                if (this->m_parser->m_started && this->m_parser->m_message_done) {
                    // should not get here
                    assert(false);
                }
                if (this->m_parser->m_started && !this->m_parser->m_message_done) {
                    // get here if other end is signlaling eom with eof
                    assert(bytes_read == 0);
                    assert(true);
                }
            } else if (bytes_read < 0) {
                if (errno_saved == EAGAIN) {
                    return XR_READER_EAGAIN;
                } else {
                    // have an io error
                    free_message(this);
                    return XR_READER_IO_ERROR;
                }
            } else /* (bytes_read > 0) */{
                IOBuffer_commit(iobuf, bytes_read);
            }
        } else {
            bytes_read = iobuf->buffer_remaining;
        }
        char* tmp = (char*)iobuf->buffer_ptr;
        char* tmp2 = (char*)iobuf->mem_p;
        ParserReturnValue ret = Parser_consume(this->m_parser, (void*) IOBuffer_data(iobuf), IOBuffer_data_len(iobuf));
        int consumed = bytes_read - ret.bytes_remaining;
        IOBuffer_consume(iobuf, consumed);
        int tmp_remaining = iobuf->buffer_remaining;
        enum ParserRC rc = ret.return_code;
        if(rc == ParserRC_end_of_data) {
            ;  // ok end to Parser_consume call - get more data
        } else if(rc == ParserRC_end_of_header) {
            ;  // ok end to Parser_consume call - get more data - in future return if reading only headers
        } else if(rc == ParserRC_end_of_message) {
            return XR_READER_EOM;
        } else if(rc == ParserRC_error) {
            free_message(this);
            return XR_READER_PARSE_ERROR;
        }

    }
}
