#include "msg_stream.h"

MsgParserRef msg_parser_new()
{
    MsgParserRef p = malloc(sizeof(MsgParser));
    msg_parser_init(p);
    return p;
}
void msg_parser_init(MsgParserRef mp)
{
    RBL_SET_TAG(MsgParser_TAG, mp)
    RBL_SET_END_TAG(MsgParser_TAG, mp)
    mp->msg_ref = message_new();
    mp->iob = IOBuffer_new(1000);
    mp->line_buffer_length = 0;
    mp->line_buffer_max = 256;
}
void msg_parser_deinit(MsgParserRef p)
{
    RBL_CHECK_TAG(MsgParser_TAG, p)
    RBL_CHECK_END_TAG(MsgParser_TAG, p)
    if(p->iob) IOBuffer_destroy(p->iob);
    if(p->msg_ref) message_deinit(p->msg_ref);
}
void msg_parser_free(MsgParserRef p)
{
    RBL_CHECK_TAG(MsgParser_TAG, p)
    RBL_CHECK_END_TAG(MsgParser_TAG, p)
    if(p->iob) IOBuffer_free(p->iob);
    if(p->msg_ref) message_free(p->msg_ref);
    free(p);
}
void msg_parser_consume(MsgParserRef mp, IOBufferRef iob_new_data, MsgParserCallback cb, void* arg)
{
    int buffer_length = IOBuffer_data_len(iob_new_data);
    char* buffer = IOBuffer_data(iob_new_data);
    int line_buffer_length;
    int consume_count = 0;
    int commit_count = 0;
    while(!IOBuffer_empty(iob_new_data)){
        char ch = IOBuffer_consume_pop_front(iob_new_data);
        if((ch == '\n')||(ch == '\0')) {

            cb(mp->msg_ref, arg, 0);
            mp->msg_ref = message_new();
        } else {
            IOBuffer_commit_push_back(mp->msg_ref->iob, ch);
        }
    }
}
