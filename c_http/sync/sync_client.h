#ifndef c_http_api_sync_client_h
#define c_http_api_sync_client_h
#define _GNU_SOURCE
#include <c_http/common/message.h>
#include <c_http/sync/sync.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


/**
 * @addtogroup group_client
 * synchronous http/1.1 Client - an opaque object that can connect to a server and make
 * one or more request roundtrip
 * @{
 */
typedef struct Client_s Client, *ClientRef;

ClientRef Client_new();
void Client_dispose(ClientRef* this_ptr);
void Client_connect(ClientRef this, char* host, int port, SyncAppMessageHandler handler);
void Client_roundtrip(ClientRef this, const char* req_buffers[], MessageRef* response);
void Client_request_round_trip(ClientRef this, MessageRef request, MessageRef* response);

/** @} */
#endif