#define _GNU_SOURCE
#include <c_eg/writer.h>
#include <c_eg/alloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <http-parser/http_parser.h>

void Wrtr_init(WrtrRef this, socket_handle_t socket)
{
    this->socket = socket;
}
WrtrRef Wrtr_new(socket_handle_t socket)
{
    WrtrRef mwref = eg_alloc(sizeof(Wrtr));
    if(mwref == NULL)
        return NULL;
    Wrtr_init(mwref, socket);
    return mwref;
}
void Wrtr_destroy(WrtrRef this)
{
}
void Wrtr_free(WrtrRef* this_ptr)
{
    WrtrRef this = *(this_ptr);
    Wrtr_destroy(this);
    eg_free(*this_ptr);
    *this_ptr = NULL;
}

void Wrtr_write(WrtrRef wrtr, MessageRef msg_ref)
{
}

void Wrtr_start(WrtrRef this, HttpStatus status, HDRListRef headers)
{
    char* first_line = NULL;
    CBufferRef cb_output_ref = NULL;
    CBufferRef serialized_headers = NULL;
    void* return_value = NULL;

    const char* reason_str = http_status_str(status);
    int len = asprintf(&first_line, "HTTP/1.1 %d %s\r\n", status, reason_str);
    if(first_line == NULL) goto failed;

    if((cb_output_ref = CBuffer_new()) == NULL) goto failed;

    serialized_headers = HDRList_serialize(headers);
    if(serialized_headers == NULL) goto failed;

    CBuffer_append(cb_output_ref, (void*)first_line, len);
    CBuffer_append(cb_output_ref, CBuffer_data(serialized_headers), CBuffer_size(serialized_headers));
    CBuffer_append_cstr(cb_output_ref, "\r\n");
    Wrtr_write_chunk(this, CBuffer_data(cb_output_ref), CBuffer_size(cb_output_ref));

//    free(first_line);
//    CBuffer_free(&serialized_headers);
//    CBuffer_free(&cb_output_ref);
    return_value = (void*)1;
    failed:
        if(first_line != NULL) free(first_line);
        if(serialized_headers != NULL) CBuffer_free(&serialized_headers);
        if(cb_output_ref != NULL) CBuffer_free(&cb_output_ref);
    return;
}
void Wrtr_write_chunk(WrtrRef this, void* buffer, int len)
{
char* c = (char*)buffer;
    printf("write_chunk this:%p, buffer: %p len: %d \n", this, buffer, len);
    int res = (int)write(this->socket, buffer, len);
//    printf("write_chunk %s %d %d \n", buffer, len, res);
    // handle error
}
void Wrtr_end(WrtrRef this)
{
}
