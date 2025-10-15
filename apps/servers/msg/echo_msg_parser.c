#include "echo_msg.h"
#include <rbl/check_tag.h>
struct EchoMsgParser_s {
    RBL_DECLARE_TAG;
    char line_buffer[1000];
    int line_buffer_length;
    int line_buffer_max;
    EchoMsgRef msg_ref;
    IOBufferRef iob;
    RBL_DECLARE_END_TAG;
};

EchoMsgParserRef echo_msg_parser_new()
{
    EchoMsgParserRef p = malloc(sizeof(EchoMsgParser));
    echo_msg_parser_init(p);
    return p;
}
void echo_msg_parser_init(EchoMsgParserRef mp)
{
    RBL_SET_TAG(EchoMsgParser_TAG, mp)
    RBL_SET_END_TAG(EchoMsgParser_TAG, mp)
    mp->msg_ref = echo_msg_new();
    mp->iob = IOBuffer_new(1000);
    mp->line_buffer_length = 0;
    mp->line_buffer_max = 256;
}
void echo_msg_parser_deinit(EchoMsgParserRef p)
{
    RBL_CHECK_TAG(EchoMsgParser_TAG, p)
    RBL_CHECK_END_TAG(EchoMsgParser_TAG, p)
    if(p->iob) IOBuffer_destroy(p->iob);
    if(p->msg_ref) echo_msg_deinit(p->msg_ref);
}
void echo_msg_parser_free(EchoMsgParserRef p)
{
    RBL_CHECK_TAG(EchoMsgParser_TAG, p)
    RBL_CHECK_END_TAG(EchoMsgParser_TAG, p)
    if(p->iob) IOBuffer_free(p->iob);
    if(p->msg_ref) echo_msg_free(p->msg_ref);
    free(p);
}
void echo_msg_parser_consume(EchoMsgParserRef mp, IOBufferRef iob_new_data, MsgParserCallback cb, void* arg)
{
    int buffer_length = IOBuffer_data_len(iob_new_data);
    char* buffer = IOBuffer_data(iob_new_data);
    int line_buffer_length;
    int consume_count = 0;
    int commit_count = 0;
    while(!IOBuffer_empty(iob_new_data)){
        char ch = IOBuffer_consume_pop_front(iob_new_data);
        if((ch == '\n')||(ch == '\0')) {

            cb(arg, mp->msg_ref, 0);
            mp->msg_ref = echo_msg_new();
        } else {
            IOBuffer_commit_push_back(echo_msg_get_content(mp->msg_ref), ch);
        }
    }
}
