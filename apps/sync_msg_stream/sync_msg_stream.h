#ifndef H_verify_client_sync_msg_stream_H
#define H_verify_client_sync_msg_stream_H

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
void sync_msg_stream_init(SyncMsgStreamRef this);
SyncMsgStreamRef sync_msg_stream_new_from_fd(int fd);
void sync_msg_stream_free(SyncMsgStreamRef this);
void sync_msg_stream_connect(SyncMsgStreamRef this, char* host, int port);
int sync_msg_stream_read(SyncMsgStreamRef client_ref, MSG_REF * msg_ref_ptr);
int sync_msg_stream_write(SyncMsgStreamRef client_ref, MSG_REF msg_ref);
void sync_msg_stream_close(SyncMsgStreamRef this);

#endif