#define _GNU_SOURCE

#include <c_http/reader.h>
#include <c_http/dsl/alloc.h>
#include <c_http/dsl/utils.h>
#include <c_http/api/iobuffer.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>

ReaderRef Reader_new(ParserRef parser, RdSocket rdsock)
{
    ReaderRef rdr = eg_alloc(sizeof(Reader));
    if(rdr == NULL)
        return NULL;
    Reader_init(rdr, parser, rdsock);
    return rdr;
}
void Reader_init(ReaderRef  this, ParserRef parser, RdSocket rdsock)
{
    ASSERT_NOT_NULL(this);
    this->m_parser = parser;
//    this->m_socket = socket;
    this->m_rdsocket = rdsock;
    this->m_iobuffer = IOBuffer_new();
}

void Reader_destroy(ReaderRef this)
{
    IOBuffer_free(&(this->m_iobuffer));

}
void Reader_free(ReaderRef* this_ptr)
{
    ReaderRef this = *this_ptr;
    Reader_destroy(this);
    eg_free((void*)this);
    *this_ptr = NULL;
}
int Reader_read(ReaderRef this, MessageRef* msgref_ptr)
{
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
                    Message_free(&(message_ptr));
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
                Message_free(&(message_ptr));
                *msgref_ptr = NULL;
                return READER_IO_ERROR;
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
        switch(ret.return_code) {
            case ParserRC_error:
                ///
                /// got a parse error - need some way to signal the caller so can send reply of bad message
                ///
                Message_free(&message_ptr);
                *msgref_ptr = NULL;
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
