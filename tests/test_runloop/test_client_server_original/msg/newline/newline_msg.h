#ifndef H_newline_msg__h
#define H_newline_msg__h
#include <runloop/runloop.h>
#include <common/list.h>
#include <tests/test_runloop/test_client_server_original/tcp/tcp_stream.h>

#define NewlineMsgParser_TAG "MSGPSR"
#define NewlineMsg_TAG "MSGTAG"


typedef struct NewlineMsg_s {
    RBL_DECLARE_TAG;
    IOBufferRef iob;
    RBL_DECLARE_END_TAG;
} NewlineMsg, * NewlineMsgRef;


typedef void(NewlineMsgParserCallback)(void* arg, NewlineMsgRef msg, int error);

typedef struct NewlineMsgParser_s {
    RBL_DECLARE_TAG;
    char line_buffer[1000];
    int line_buffer_length;
    int line_buffer_max;
    NewlineMsgRef msg_ref;
    IOBufferRef iob;
    NewlineMsgParserCallback* cb;
    void* cb_arg;
    RBL_DECLARE_END_TAG;
} NewlineMsgParser, *NewlineMsgParserRef;


NewlineMsgRef newline_msg_new();
void newline_msg_init(NewlineMsgRef msg_ref);
void newline_msg_deinit(NewlineMsgRef msg_ref);
void newline_msg_free(NewlineMsgRef msg_ref);
IOBufferRef newline_msg_get_content(NewlineMsgRef msg);
void newline_msg_set_content(NewlineMsgRef msg, IOBufferRef iob);
IOBufferRef newline_msg_serialize(NewlineMsgRef mr);

NewlineMsgParserRef newline_msg_parser_new(NewlineMsgParserCallback* cb, void* arg);
void newline_msg_parser_init(NewlineMsgParserRef parser_ref, NewlineMsgParserCallback* cb, void* arg);
void newline_msg_parser_deinit(NewlineMsgParserRef parser_ref);
void newline_msg_parser_free(NewlineMsgParserRef parser_ref);
int newline_msg_parser_consume(NewlineMsgParserRef, IOBufferRef new_data);
const char* newline_msg_parser_strerror(NewlineMsgParserRef parser_ref, int parser_errno);
#endif