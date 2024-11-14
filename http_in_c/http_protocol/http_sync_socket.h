#ifndef c_http_demo_syncsocket_h
#define c_http_demo_syncsocket_h

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
typedef struct DemoSyncSocket_s DemoSyncSocket, *DemoSyncSocketRef;

/**
 * @brief Create a new client instance.
 * @return DemoSyncSocketRef
 */
DemoSyncSocketRef demo_syncsocket_new();
void demo_syncsocket_init(DemoSyncSocketRef this);
DemoSyncSocketRef demo_syncsocket_new_from_fd(int fd);

/**
 * @brief Free ca client instance and all it associated resources, including closing the socket connection to the server.
 *
 * @NOTE: This function updates the variable holding the DemoSyncSocketRef to NULL.
 *
 * @param this_ptr DemoSyncSocketRef*
 */
void demo_syncsocket_free(DemoSyncSocketRef this);
/**
 * @brief Connecto the the given host and port.
 * @param this DemoSyncSocketRef
 * @param host char*      A host name like "localhost" or "google.com"
 * @param port            A port number
 */
void demo_syncsocket_connect(DemoSyncSocketRef this, char* host, int port);
int demo_syncsocket_read_message(DemoSyncSocketRef client_ref, DemoMessageRef* msg_ref_ptr);
int demo_syncsocket_write_message(DemoSyncSocketRef client_ref, DemoMessageRef msg_ref);
void demo_syncsocket_close(DemoSyncSocketRef this);

/** @} */
#endif