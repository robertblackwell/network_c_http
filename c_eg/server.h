#ifndef c_ceg_server_h
#define c_ceg_server_h

#include <c_eg/constants.h>
#include <c_eg/queue.h>
#include <c_eg/worker.h>
#include <c_eg/socket_functions.h>

struct Server_s {
    int                     port;
    socket_handle_t         socket_fd;
    int                     nbr_workers;
    QueueRef                qref;
    WorkerRef               worker_tab[NBR_WORKERS];
};
typedef struct  Server_s Server, *ServerRef;

ServerRef Server_new(int port);
void Server_free(ServerRef* srefptr);
void Server_listen(ServerRef server);
void Server_terminate(ServerRef this);
#endif