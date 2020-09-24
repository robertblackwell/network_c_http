#define _GNU_SOURCE
#include <c_eg/message.h>
#include <c_eg/reader.h>
#include <c_eg/writer.h>

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

struct Client_s {
    socket_handle_t sock;
    WrtrRef wrtr;
    RdrRef  rdr;
    ParserRef parser;
};
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
void Client_roundtrip(ClientRef this, char* req_buffers[], MessageRef* response);
