#ifndef c_http_server_h
#define c_http_server_h
/**
 * @defgroup group_server Server
 * @brief Implements a multi-threaded synchronous http/1.1 server
 * @{
 */

#include <c_http/api/handler.h>


/**
 * Server - a synchronous server as an opaque object
 */
typedef struct  Server_s Server, *ServerRef;

/**
 * @brief Create a new server object.
 * @param port     int              The localhost:port on which the server will listen
 * @param handler  HandlerFunction  A function conforming to HandlerFunction (see handler.h) which will be called to handle all requests that are parsed successfully.
 * @return ServerRef
 */
ServerRef Server_new(int port, int nbr_threads, HandlerFunction handler);
/**
 * @brief Free a ServerRef and all its associated resources.
 *
 * @Note: This function updates the variable holding the ServerRef to NULL
 *
 * @param srefptr ServerRef*
 */
void Server_free(ServerRef* srefptr);
/**
 * @brief Start listening on the servers port for incoming connections.
 *
 * @param server ServerRef
 */
void Server_listen(ServerRef server);
/**
 * @brief terminate the server. Which requires terminating all the worker threads.
 *
 * @param this ServerRef
 */
void Server_terminate(ServerRef this);

/** @} */
#endif