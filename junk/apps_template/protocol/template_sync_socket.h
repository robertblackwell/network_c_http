#ifndef c_http_tmpl_syncsocket_h
#define c_http_tmpl_syncsocket_h

#include <src/tmpl_protocol/tmpl_message.h>
#include <src/tmpl_protocol/tmpl_message_parser.h>

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
typedef struct TmplSyncSocket_s TmplSyncSocket, *TmplSyncSocketRef;

/**
 * @brief Create a new client instance.
 * @return TmplSyncSocketRef
 */
TmplSyncSocketRef tmpl_syncsocket_new();
void tmpl_syncsocket_init(TmplSyncSocketRef this);
TmplSyncSocketRef tmpl_syncsocket_new_from_fd(int fd);

/**
 * @brief Free ca client instance and all it associated resources, including closing the socket connection to the server.
 *
 * @NOTE: This function updates the variable holding the TmplSyncSocketRef to NULL.
 *
 * @param this_ptr TmplSyncSocketRef*
 */
void tmpl_syncsocket_free(TmplSyncSocketRef this);
/**
 * @brief Connecto the the given host and port.
 * @param this TmplSyncSocketRef
 * @param host char*      A host name like "localhost" or "google.com"
 * @param port            A port number
 */
void tmpl_syncsocket_connect(TmplSyncSocketRef this, char* host, int port);
int tmpl_syncsocket_read_message(TmplSyncSocketRef client_ref, TmplMessageRef* msg_ref_ptr);
int tmpl_syncsocket_write_message(TmplSyncSocketRef client_ref, TmplMessageRef msg_ref);
void tmpl_syncsocket_close(TmplSyncSocketRef this);

/** @} */
#endif