#include <c_http/async/async_server.h>
#include <c_http/common/message.h>

#include <stdio.h>
#include <mcheck.h>
#include<signal.h>

AsyncServerRef g_sref;
int handler_example(MessageRef request, void* wrtr)
{
}
void sig_handler(int signo)
{
    printf("app.c signal handler \n");
    if (signo == SIGINT) {
        printf("received SIGINT\n");
        AsyncServer_terminate( g_sref);
    }
}

int main()
{
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        printf("app.c main signal() failed");
    }
    printf("Hello this is xr-junk main \n");
    AsyncServerRef sref = AsyncServer_new(9001);
    g_sref = sref;
    AsyncServer_listen(sref);
    AsyncServer_dispose(&sref);

}

