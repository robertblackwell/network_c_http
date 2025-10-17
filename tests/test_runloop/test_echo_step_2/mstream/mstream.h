#ifndef H_tests_test_runloop_test_echo_STREAM_H
#define H_tests_test_runloop_test_echo_STREAM_H
#include <msg/msg_selection_header.h>
typedef struct MStream_s MStreamn, *MStreamRef;
MStreamRef mstream_new(const char* host, int port);
MStreamRef mstream_from_fd(int fd);
void mstream_write(MStreamRef stream, MSG_REF msg_ref);
MSG_REF mstream_read(MStreamRef stream, MSG_PARSER_REF parser);
void mstream_free(MStreamRef s);
#endif