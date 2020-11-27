#ifndef c_http_server_h
#define c_http_server_h

#include <c_http/constants.h>
#include <c_http/queue.h>
#include <c_http/sync/worker.h>
#include <c_http/message.h>
#include <c_http/sync/writer.h>
#include <c_http/socket_functions.h>
#include <c_http/sync/handler.h>
#include <c_http/sync/handler_example.h>

struct Server_s {
    int                     port;
    socket_handle_t         socket_fd;
    int                     nbr_workers;
    HandlerFunction         handler;
    QueueRef                qref;
    WorkerRef               worker_tab[NBR_WORKERS];
};
typedef struct  Server_s Server, *ServerRef;

/**
 * Create a new server object.
 * \param port     The localhost:port on which the server will listen
 * \param handler  A function conforming to HandlerFunction (see handler.h) which will be called to handle all requests that are parsed successfully.
 * \return
 */
ServerRef Server_new(int port, HandlerFunction handler);
void Server_free(ServerRef* srefptr);
void Server_listen(ServerRef server);
void Server_terminate(ServerRef this);
#endif