#include "newline_msg.h"
#include <rbl/check_tag.h>
struct NewLineMsgParser_s {
    RBL_DECLARE_TAG;
    char line_buffer[1000];
    int line_buffer_length;
    int line_buffer_max;
    NewLineMsgRef msg_ref;
    IOBufferRef iob;
    RBL_DECLARE_END_TAG;
};

NewLineMsgParserRef newline_msg_parser_new()
{
    NewLineMsgParserRef p = malloc(sizeof(NewLineMsgParser));
    newline_msg_parser_init(p);
    return p;
}
void newline_msg_parser_init(NewLineMsgParserRef mp)
{
    RBL_SET_TAG(NewLineMsgParser_TAG, mp)
    RBL_SET_END_TAG(NewLineMsgParser_TAG, mp)
    mp->msg_ref = newline_msg_new();
    mp->iob = IOBuffer_new(1000);
    mp->line_buffer_length = 0;
    mp->line_buffer_max = 256;
}
void newline_msg_parser_deinit(NewLineMsgParserRef p)
{
    RBL_CHECK_TAG(NewLineMsgParser_TAG, p)
    RBL_CHECK_END_TAG(NewLineMsgParser_TAG, p)
    if(p->iob) IOBuffer_destroy(p->iob);
    if(p->msg_ref) newline_msg_deinit(p->msg_ref);
}
void newline_msg_parser_free(NewLineMsgParserRef p)
{
    RBL_CHECK_TAG(NewLineMsgParser_TAG, p)
    RBL_CHECK_END_TAG(NewLineMsgParser_TAG, p)
    if(p->iob) IOBuffer_free(p->iob);
    if(p->msg_ref) newline_msg_free(p->msg_ref);
    free(p);
}
void newline_msg_parser_consume(NewLineMsgParserRef mp, IOBufferRef iob_new_data, NewLineMsgParserCallback cb, void* arg)
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
            mp->msg_ref = newline_msg_new();
        } else {
            IOBuffer_commit_push_back(newline_msg_get_content(mp->msg_ref), ch);
        }
    }
}
