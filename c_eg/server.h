#ifndef c_ceg_server_h
#define c_ceg_server_h

#include <c_eg/constants.h>
#include <c_eg/queue.h>
#include <c_eg/worker.h>
#include <c_eg/message.h>
#include <c_eg/writer.h>
#include <c_eg/socket_functions.h>
#include <c_eg/handler.h>
#include <c_eg/handler_example.h>

struct Server_s {
    int                     port;
    socket_handle_t         socket_fd;
    int                     nbr_workers;
    HandlerFunction         handler;
    Queue*                  qref;
    Worker*                 worker_tab[NBR_WORKERS];
};
typedef struct  Server_s Server;

/**
 * Create a new server object.
 * \param port     The localhost:port on which the server will listen
 * \param handler  A function conforming to HandlerFunction (see handler.h) which will be called to handle all requests that are parsed successfully.
 * \return
 */
Server* Server_new(int port, HandlerFunction handler);
void Server_free(Server** srefptr);
void Server_listen(Server* server);
void Server_terminate(Server* this);
#endif