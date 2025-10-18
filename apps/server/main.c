#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <interfaces/server_app_interface.h>
#include <server/server_ctx.h>
#include <apps/servers/server_apps/echo_app.h>
#ifdef __cplusplus
extern "C" {
#endif

ServerCtx server_ctx;
static void server_main(RunloopRef runloop, void* arg);
static void* accept_cb(void* arg, int sock, int error);

int main() 
{
    int port = 9002;
    int fd = local_create_bound_socket(port, "localhost");
    socket_set_non_blocking(fd);
    RunloopRef runloop = runloop_new();
    ServerCtxRef server_ctx_ref = &server_ctx;
    server_ctx_init(server_ctx_ref, runloop, fd, echo_app_get_server_app_interface());
    server_ctx_run(server_ctx_ref);
    runloop_run(runloop, 0L);
    return 0;
}
#ifdef __cplusplus
}
#endif
