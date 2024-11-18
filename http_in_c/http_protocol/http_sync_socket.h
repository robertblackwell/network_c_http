#ifndef c_http_http_syncsocket_h
#define c_http_http_syncsocket_h

#include <http_in_c/http/http_message.h>
#include <http_in_c/http/parser.h>

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
typedef struct HttpSyncSocket_s HttpSyncSocket, *HttpSyncSocketRef;

/**
 * @brief Create a new client instance.
 * @return HttpSyncSocketRef
 */
HttpSyncSocketRef http_syncsocket_new();
void http_syncsocket_init(HttpSyncSocketRef this);
HttpSyncSocketRef http_syncsocket_new_from_fd(int fd);

/**
 * @brief Free ca client instance and all it associated resources, including closing the socket connection to the server.
 *
 * @NOTE: This function updates the variable holding the HttpSyncSocketRef to NULL.
 *
 * @param this_ptr HttpSyncSocketRef*
 */
void http_syncsocket_free(HttpSyncSocketRef this);
/**
 * @brief Connecto the the given host and port.
 * @param this HttpSyncSocketRef
 * @param host char*      A host name like "localhost" or "google.com"
 * @param port            A port number
 */
void http_syncsocket_connect(HttpSyncSocketRef this, char* host, int port);
int http_syncsocket_read_message(HttpSyncSocketRef client_ref, HttpMessageRef* msg_ref_ptr);
int http_syncsocket_write_message(HttpSyncSocketRef client_ref, HttpMessageRef msg_ref);
void http_syncsocket_close(HttpSyncSocketRef this);

/** @} */
#endif