#ifndef c_http_demo_client_h
#define c_http_demo_client_h

#include <http_in_c/demo_protocol/demo_message.h>
#include <http_in_c/demo_protocol/demo_parser.h>

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
void democlient_init(DemoClientRef this);

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
int democlient_read_message(DemoClientRef client_ref, DemoMessageRef* msg_ref_ptr);
int democlient_write_message(DemoClientRef client_ref, DemoMessageRef msg_ref);


/** @} */
#endif