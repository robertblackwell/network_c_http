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
    QueueRef                qref;
    WorkerRef               worker_tab[NBR_WORKERS];
};
typedef struct  Server_s Server, *ServerRef;

ServerRef Server_new(int port, HandlerFunction handler);
void Server_free(ServerRef* srefptr);
void Server_listen(ServerRef server);
void Server_terminate(ServerRef this);
#endif