#define _GNU_SOURCE

#include <c_eg/reader.h>
#include <c_eg/alloc.h>
#include <c_eg/utils.h>
#include <c_eg/buffer/iobuffer.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>

RdrRef Rdr_new(ParserRef parser, RdSocket rdsock)
{
    RdrRef rdr = eg_alloc(sizeof(Rdr));
    if(rdr == NULL)
        return NULL;
    Rdr_init(rdr, parser, rdsock);
    return rdr;
}
void Rdr_init(RdrRef  this, ParserRef parser, RdSocket rdsock)
{
    ASSERT_NOT_NULL(this);
    this->m_parser = parser;
//    this->m_socket = socket;
    this->m_rdsocket = rdsock;
    this->m_iobuffer = IOBuffer_new();
}

void Rdr_destroy(RdrRef this)
{
    IOBuffer_free(&(this->m_iobuffer));

}
void Rdr_free(RdrRef* this_ptr)
{
    RdrRef this = *this_ptr;
    Rdr_destroy(this);
    eg_free((void*)this);
    *this_ptr = NULL;
}
int Rdr_read(RdrRef this, MessageRef* msgref_ptr)
{
    IOBufferRef iobuf = this->m_iobuffer;
    MessageRef message_ptr = Message_new();
    Parser_begin(this->m_parser, message_ptr);
    int bytes_read;
    for(;;) {
        if(IOBuffer_data_len(iobuf) == 0 ) {
            IOBuffer_reset(iobuf);
            void* buf = IOBuffer_space(iobuf); int len = IOBuffer_space_len(iobuf);
            void* c = this->m_rdsocket.ctx;
            ReadFunc rf = (this->m_rdsocket.read_f);
            bytes_read = RdSocket_read(&(this->m_rdsocket), buf, len);
            // bytes_read == 0 means other end closed the socket, if have already started but not finished a message
            // send 0 bytes to parser to signal EOF
            //
            // bytes_read < 0 means io error which includes I closed the socket on the last iteration of the read message loop
            // in either case return NULL unless the other end is signalling end of message with a close (non HTTP spec compliant)
            if(bytes_read == 0) {
                if((bytes_read == 0) && this->m_parser->m_started && (!this->m_parser->m_message_done)) {
                    bytes_read = 0;
                } else {
                    Message_free(&(message_ptr));
                    *msgref_ptr = NULL;
                    int er = errno;
                    return 0;
                }
            } else if (bytes_read < 0) {
                int x = errno;
                Message_free(&(message_ptr));
                *msgref_ptr = NULL;
                // collect errno for more details
                return RDR_IO_ERROR;
            }
            IOBuffer_commit(iobuf, bytes_read);
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
                printf("Got error from parser %d\n", ret.return_code);
                ParserError pe = Parser_get_error(this->m_parser);
                printf("Error details %s %s \n", pe.m_name, pe.m_description);
//                assert(false);
                Message_free(&message_ptr);
                *msgref_ptr = NULL;
                return RDR_PARSE_ERROR;
                break;
            case ParserRC_end_of_data:
                break;
            case ParserRC_end_of_header:
                break;
            case ParserRC_end_of_message:
                *msgref_ptr = message_ptr;
                // return ok
                return RDR_OK;
        }
    }
}
