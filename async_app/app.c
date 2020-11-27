#include <c_http/xr/xr_server.h>
#include <c_http/message.h>

#include <stdio.h>
#include <mcheck.h>
#include<signal.h>

XrServerRef g_sref;
int handler_example(MessageRef request, void* wrtr)
{
}
void sig_handler(int signo)
{
    printf("app.c signal handler \n");
    if (signo == SIGINT) {
        printf("received SIGINT\n");
        XrServer_terminate( g_sref);
    }
}

int main()
{
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        printf("app.c main signal() failed");
    }
    printf("Hello this is xr main \n");
    XrServerRef sref = XrServer_new(9001, handler_example);
    g_sref = sref;
    XrServer_listen(sref);
    XrServer_free(&sref);

}

