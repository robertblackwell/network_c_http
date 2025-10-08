#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "stream_ctx.h"

int main() 
{
    int port = 9002;
    int fd = local_create_bound_socket(port, "localhost");
    socket_set_non_blocking(fd);
    ListenerCtxRef server = listener_ctx_new(fd, 1, runloop_new());
    listener_ctx_run(server);
    return 0;

}

