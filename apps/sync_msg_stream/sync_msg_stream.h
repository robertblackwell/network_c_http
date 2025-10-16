#ifndef H_apps_sync_msg_stream_sync_msg_stream_H
#define H_apps_sync_msg_stream_sync_msg_stream_H

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <msg/msg_selection_header.h>
typedef struct SyncMsgStream_s SyncMsgStream, *SyncMsgStreamRef;

SyncMsgStreamRef sync_msg_stream_new(MSG_PARSER_REF parser_ref);
void sync_msg_stream_init(SyncMsgStreamRef stream, MSG_PARSER_REF parser_ref);
SyncMsgStreamRef sync_msg_stream_new_from_fd(MSG_PARSER_REF parser_ref, int fd);
void sync_msg_stream_free(SyncMsgStreamRef stream);
void sync_msg_stream_connect(SyncMsgStreamRef stream, char* host, int port);
int sync_msg_stream_read(SyncMsgStreamRef stream_, MSG_REF * msg_ref_ptr);
int sync_msg_stream_write(SyncMsgStreamRef stream, MSG_REF msg_ref);
void sync_msg_stream_free(SyncMsgStreamRef stream);

#endif