#ifndef c_http_sync_server_h
#define c_http_sync_server_h
/**
 * @defgroup group_server Server
 * @brief Implements a multi-threaded synchronous http/1.1 server
 * @{
 */

#include <c_http/sync/sync_handler.h>


/**
 * Server - a synchronous server as an opaque object
 */
typedef struct  SyncServer_s Server, *SyncServerRef;

/**
 * @brief Create a new server object.
 * @param port     int              The localhost:port on which the server will listen
 * @param handler  SyncHandlerFunction  A function conforming to SyncHandlerFunction (see handler.h) which will be called to handle all requests that are parsed successfully.
 * @return SyncServerRef
 */
SyncServerRef SyncServer_new(int port, int nbr_threads, SyncHandlerFunction handler);
/**
 * @brief Free a SyncServerRef and all its associated resources.
 *
 * @Note: This function updates the variable holding the SyncServerRef to NULL
 *
 * @param srefptr SyncServerRef*
 */
void SyncServer_dispose(SyncServerRef* srefptr);
/**
 * @brief Start listening on the servers port for incoming connections.
 *
 * @param server SyncServerRef
 */
void SyncServer_listen(SyncServerRef server);
/**
 * @brief terminate the server. Which requires terminating all the worker threads.
 *
 * @param this SyncServerRef
 */
void SyncServer_terminate(SyncServerRef this);

/** @} */
#endif