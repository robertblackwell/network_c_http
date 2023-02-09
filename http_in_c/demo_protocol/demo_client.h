#ifndef c_http_demo_client_h
#define c_http_demo_client_h
#define _GNU_SOURCE
#include <http_in_c/demo_protocol/demo_message.h>

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
typedef struct DemoClient_s DemoClient, *DemoClientRef;

/**
 * @brief Create a new client instance.
 * @return DemoClientRef
 */
DemoClientRef democlient_new();
/**
 * @brief Free ca client instance and all it associated resources, including closing the socket connection to the server.
 *
 * @NOTE: This function updates the variable holding the DemoClientRef to NULL.
 *
 * @param this_ptr DemoClientRef*
 */
void democlient_dispose(DemoClientRef* this_ptr);

/**
 * @brief Connecto the the given host and port.
 * @param this DemoClientRef
 * @param host char*      A host name like "localhost" or "google.com"
 * @param port            A port number
 */
void democlient_connect(DemoClientRef this, char* host, int port);

/**
 * @brief Sends a request to a server and waits for a response
 * \param this        DemoClientRef
 * \param req_buffers The Request as a c-array of char* terminated by NULL
 * \param response    MessageRef
 */
void democlient_roundtrip(DemoClientRef this, const char* req_buffers[], DemoMessageRef* response);

/**
 * @brief Sends a request to a server and waits for a response
 * \param this        DemoClientRef
 * \param request     MessageRef  The Request as MessageRef instance
 * \param response    MessageRef
 */
void democlient_request_round_trip(DemoClientRef this, DemoMessageRef request, DemoMessageRef* response);

/** @} */
#endif