#ifndef c_http_tcp_sync_stream_h
#define c_http_tcp_sync_stream_h

#include <src/demo_protocol/message.h>
#include <src/demo_protocol/message_parser.h>

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

typedef struct DemoSyncSocket_s DemoSyncSocket, *DemoSyncSocketRef;

DemoSyncSocketRef tcp_sync_stream_new();
void tcp_sync_stream_init(DemoSyncSocketRef this);
DemoSyncSocketRef tcp_sync_stream_new_from_fd(int fd);

void tcp_sync_stream_free(DemoSyncSocketRef this);
void tcp_sync_stream_connect(DemoSyncSocketRef this, char* host, int port);
int tcp_sync_stream_read_message(DemoSyncSocketRef client_ref, MessageRef* msg_ref_ptr);
int tcp_sync_stream_write_message(DemoSyncSocketRef client_ref, MessageRef msg_ref);
void tcp_sync_stream_close(DemoSyncSocketRef this);

#endif