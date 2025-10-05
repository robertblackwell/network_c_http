//
// Created by robert on 11/8/24.
//
#include "fill_iobuffer.h"
IOBufferRef fill_iobuffer(char* line, size_t max_len, size_t required_data_length)
{
    IOBufferRef iob = IOBuffer_new_with_capacity((int)max_len);
    char* buffer = IOBuffer_space(iob);
    unsigned long line_length = strlen(line);
//    IOBuffer_data_add(iob, "\x02", 1);
    int line_len = (int)strlen(line);
    while(true) {
        IOBuffer_data_add(iob, line, line_len);
        if(IOBuffer_data_len(iob) >= required_data_length+1) {
            break;
        }
    }
//    IOBuffer_data_add(iob, "\x03", 1);

    return iob;
}
DemoMessageRef fill_demo_message(char* line, size_t max_len, size_t required_data_length)
{
    BufferChainRef bc = BufferChain_new();
    DemoMessageRef msg = demo_message_new();
    BufferChain_append_IOBuffer(bc, fill_iobuffer(line, max_len, required_data_length));
    demo_message_set_body(msg, bc);
    return msg;
}
