#include <c_eg/server.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <errno.h>

#include <c_eg/constants.h>
#include <c_eg/utils.h>

#include <c_eg/logger.h>
#include <c_eg/socket_functions.h>
#include <c_eg/queue.h>
#include <c_eg/worker.h>


//
// create a listening socket from host and port
//
socket_handle_t create_listener_socket(int port, const char* host)
{

    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    socket_handle_t tmp_socket;
    sin.sin_family = AF_INET; // or AF_INET6 (address family)
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = inet_addr("127.0.0.1");
    int result;
    int yes = 1;

    if( (tmp_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1 ) 
        goto error_01;

    // sin.sin_len = sizeof(sin);
    if( (result = setsockopt(tmp_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))) != 0 )
        goto error_02;

    if( (result = bind(tmp_socket, (struct sockaddr *)&sin, sizeof(sin))) != 0)
        goto error_03;

    if((result = listen(tmp_socket, SOMAXCONN)) != 0)
        goto error_04;
    return tmp_socket;

    error_01:
        printf("socket call failed with errno %d \n", errno); 
        assert(0);
    error_02:
        printf("setsockopt call failed with errno %d \n", errno); 
        assert(0);
    error_03:
        printf("bind call failed with errno %d \n", errno); 
        assert(0);
    error_04:
        printf("listen call failed with errno %d \n", errno); 
        assert(0);
}

struct Server_s {
    int                     port;
    int                     nbr_workers;
    QueueRef                qref;
    WorkerRef               worker_tab[NBR_WORKERS];
};


ServerRef Server_new(int port)
{
    ServerRef sref = (ServerRef)malloc(sizeof(Server));
    sref->nbr_workers = NBR_WORKERS;
    sref->port = port;
    sref->qref = Queue_new();
    return sref;
}

void Server_free(ServerRef* sref)
{
    ASSERT_NOT_NULL(*sref);
    free(*sref);
    *sref = NULL;
}

void Server_listen(ServerRef sref)
{
    ASSERT_NOT_NULL(sref)
    printf("Server_listen\n");
    //
    // Start the worker threads
    //
    for(int i = 0; i < sref->nbr_workers; i++)
    {
        WorkerRef wref = Worker_new(sref->qref, i);
        sref->worker_tab[i] = wref;
        Worker_start(wref);
    }
    printf("workers started\n");
    //
    // Start a Monitor thread that will check the workers are not zombies
    //
    // Monitor   monitor{worker_v,  (int)nbr_workers};
    // monitor.start();

    // now listen  for incoming connections
    {
        int port = sref->port;
        struct sockaddr_in peername;
        unsigned int addr_length = (unsigned int)sizeof(peername);
        int socket_fd = create_listener_socket(port, "127.0.0.1");
        for(;;)
        {
            int sock2 = accept(socket_fd, (struct sockaddr*)&peername, &addr_length);
            if( sock2 <= 0 )
            {
                LOG_FMT("%s %d", "Listener thread :: accept failed terminating sock2 : ", sock2);
                break;
            }
            printf("Server_listener adding socket to qref %d \n", sock2);
            Queue_add(sref->qref, sock2);
        }
    }
    //
    // wait for the workers to complete
    //
    for(int i = 0; i < sref->nbr_workers; i++) {
        WorkerRef wref = sref->worker_tab[i];
        pthread_join( *Worker_pthread(wref), NULL);
    }
    // also wait for the monitor  to complete
    
}


