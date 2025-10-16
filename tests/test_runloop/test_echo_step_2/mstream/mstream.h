#ifndef H_tests_test_runloop_test_echo_STREAM_H
#define H_tests_test_runloop_test_echo_STREAM_H
#include <msg/newline_msg.h>
typedef struct MStream_s MStreamn, *MStreamRef;
MStreamRef mstream_new(const char* host, int port);
MStreamRef mstream_from_fd(int fd);
void mstream_write(MStreamRef stream, NewLineMsgRef newline_msg);
NewLineMsgRef mstream_read(MStreamRef stream, NewLineMsgParserRef parser);
void mstream_free(MStreamRef s);
#endif