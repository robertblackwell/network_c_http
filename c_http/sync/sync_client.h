#ifndef c_http_api_sync_client_h
#define c_http_api_sync_client_h
#define _GNU_SOURCE
#include <c_http/common/message.h>

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

/**
 * @brief Create a new client instance.
 * @return ClientRef
 */
ClientRef Client_new();
/**
 * @brief Free ca client instance and all it associated resources, including closing the socket connection to the server.
 *
 * @NOTE: This function updates the variable holding the ClientRef to NULL.
 *
 * @param this_ptr ClientRef*
 */
void Client_dispose(ClientRef* this_ptr);

/**
 * @brief Connecto the the given host and port.
 * @param this ClientRef
 * @param host char*      A host name like "localhost" or "google.com"
 * @param port            A port number
 */
void Client_connect(ClientRef this, char* host, int port);

/**
 * @brief Sends a request to a server and waits for a response
 * \param this        ClientRef
 * \param req_buffers The Request as a c-array of char* terminated by NULL
 * \param response    MessageRef
 */
void Client_roundtrip(ClientRef this, const char* req_buffers[], MessageRef* response);

/**
 * @brief Sends a request to a server and waits for a response
 * \param this        ClientRef
 * \param request     MessageRef  The Request as MessageRef instance
 * \param response    MessageRef
 */
void Client_request_round_trip(ClientRef this, MessageRef request, MessageRef* response);

/** @} */
#endif