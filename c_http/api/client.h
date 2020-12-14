#define _GNU_SOURCE
#include <c_http/api/message.h>

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define TYPE Client
#define Client_TAG "CLIENT"
#include <c_http/check_tag.h>
#undef TYPE
#define CLIENT_DECLARE_TAG DECLARE_TAG(Client)
#define CLIENT_CHECK_TAG(p) CHECK_TAG(Client, p)
#define CLIENT_SET_TAG(p) SET_TAG(Client, p)


/**
 * synchronous http/1.1 Client - an opaque object that can connect to a server make a request roundtrip
 */
typedef struct Client_s Client, *ClientRef;

ClientRef Client_new();
void Client_free(ClientRef* this_ptr);
void Client_connect(ClientRef this, char* host, int port);

/**
 * Sends a request to a server and waits for a response
 * \param this        ClientRef
 * \param req_buffers The Request as a c-array of char* terminated by NULL
 * \param response    MessageRef
 */
void Client_roundtrip(ClientRef this, const char* req_buffers[], MessageRef* response);
void Client_request_round_trip(ClientRef this, MessageRef request, MessageRef* response);