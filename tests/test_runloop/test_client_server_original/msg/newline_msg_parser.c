#include "newline_msg.h"

NewlineMsgParserRef newline_msg_parser_new(NewlineMsgParserCallback* cb, void* arg)
{
    NewlineMsgParserRef p = malloc(sizeof(NewlineMsgParser));
    newline_msg_parser_init(p, cb, arg);
    return p;
}
void newline_msg_parser_init(NewlineMsgParserRef mp, NewlineMsgParserCallback* cb, void* arg)
{
    RBL_SET_TAG(NewlineMsgParser_TAG, mp)
    RBL_SET_END_TAG(NewlineMsgParser_TAG, mp)
    mp->msg_ref = newline_msg_new();
    mp->iob = IOBuffer_new(1000);
    mp->line_buffer_length = 0;
    mp->line_buffer_max = 256;
    assert(cb != NULL);
    mp->cb = cb;
    mp->cb_arg = arg;
}
void newline_msg_parser_deinit(NewlineMsgParserRef p)
{
    RBL_CHECK_TAG(NewlineMsgParser_TAG, p)
    RBL_CHECK_END_TAG(NewlineMsgParser_TAG, p)
    if(p->iob) IOBuffer_destroy(p->iob);
    if(p->msg_ref) newline_msg_deinit(p->msg_ref);
}
void newline_msg_parser_free(NewlineMsgParserRef p)
{
    RBL_CHECK_TAG(NewlineMsgParser_TAG, p)
    RBL_CHECK_END_TAG(NewlineMsgParser_TAG, p)
    if(p->iob) IOBuffer_free(p->iob);
    if(p->msg_ref) newline_msg_free(p->msg_ref);
    free(p);
}
void newline_msg_parser_consume(NewlineMsgParserRef mp, IOBufferRef iob_new_data)
{
    int buffer_length = IOBuffer_data_len(iob_new_data);
    char* buffer = IOBuffer_data(iob_new_data);
    int line_buffer_length;
    int consume_count = 0;
    int commit_count = 0;
    while(!IOBuffer_empty(iob_new_data)){
        char ch = IOBuffer_consume_pop_front(iob_new_data);
        if((ch == '\n')||(ch == '\0')) {

            mp->cb(mp->cb_arg, mp->msg_ref, 0);
            mp->msg_ref = newline_msg_new();
        } else {
            IOBuffer_commit_push_back(mp->msg_ref->iob, ch);
        }
    }
}
