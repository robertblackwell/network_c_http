#ifndef H_sync_msg_stream_H
#define H_sync_msg_stream_H
#include <pthread.h>
#ifdef APPLE_FLAG
#include <sys/_pthread/_pthread_t.h>
#endif
#include <common/socket_functions.h>
#include <rbl/unittest.h>
#include <apps/msg/msg_generic.h>

typedef struct SyncMsgStream_s  SyncMsgStream, *SyncMsgStreamRef;
SyncMsgStreamRef sync_msg_stream_new();
void sync_msg_stream_free(SyncMsgStreamRef s);
void sync_msg_stream_connect(SyncMsgStreamRef sync_stream, int port);
void sync_msg_stream_send(SyncMsgStreamRef sync_stream, GenericMsgRef msgref);
GenericMsgRef sync_msg_stream_recv(SyncMsgStreamRef sync_stream);
#endif