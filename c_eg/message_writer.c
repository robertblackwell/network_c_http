#define _GNU_SOURCE
#include <c_eg/message_writer.h>
#include <c_eg/alloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <http-parser/http_parser.h>

void MessageWriter_init(MessageWriterRef this, socket_handle_t socket)
{
    this->socket = socket;
}
MessageWriterRef MessageWriter_new(socket_handle_t socket)
{
    MessageWriterRef mwref = eg_alloc(sizeof(MessageWriter));
    if(mwref == NULL)
        return NULL;
    MessageWriter_init(mwref, socket);
    return mwref;
}
void MessageWriter_destroy(MessageWriterRef this)
{
}
void MessageWriter_free(MessageWriterRef* this_ptr)
{
    MessageWriterRef this = *(this_ptr);
    MessageWriter_destroy(this);
    eg_free(*this_ptr);
    *this_ptr = NULL;
}

void MessageWriter_write(MessageWriterRef wrtr, MessageRef msg_ref)
{
}

void MessageWriter_start(MessageWriterRef this, HttpStatus status, HDRListRef headers)
{
    const char* reason_str = http_status_str(status);
    char* first_line = NULL;
    int len = asprintf(&first_line, "HTTP/1.1 %d %s\r\n", status, reason_str);
    if(first_line == NULL) goto failed;

    CBufferRef cb_output_ref = NULL;
    if((cb_output_ref = CBuffer_new()) == NULL) goto failed;
    CBufferRef serialized_headers = NULL;
    serialized_headers = HDRList_serialize(headers);

    CBuffer_append(cb_output_ref, (void*)first_line, len);
    /// this is clumsy - change HDRList_serialize() to deposit into an existing ContigBuffer
    CBuffer_append(cb_output_ref, CBuffer_data(serialized_headers), CBuffer_size(serialized_headers));
    CBuffer_append_cstr(cb_output_ref, "\r\n");
    int x = len+2;
    MessageWriter_write_chunk(this, CBuffer_data(cb_output_ref), CBuffer_size(cb_output_ref));

    free(first_line);
    CBuffer_free(&serialized_headers);
    CBuffer_free(&cb_output_ref);
    return;
    failed:
        if(first_line != NULL) free(first_line);
        if(serialized_headers != NULL) CBuffer_free(&serialized_headers);
        if(cb_output_ref != NULL) CBuffer_free(&cb_output_ref);

}
void MessageWriter_write_chunk(MessageWriterRef this, void* buffer, int len)
{
char* c = (char*)buffer;
    printf("write_chunk this:%p, buffer: %p len: %d \n", this, buffer, len);
    int res = (int)write(this->socket, buffer, len);
    // handle error
}
void MessageWriter_end(MessageWriterRef this)
{
}
