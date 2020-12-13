#include <c_http/aio_api/xr_server.h>
#include <c_http/api/message.h>

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
    printf("Hello this is xr-junk main \n");
    XrServerRef sref = XrServer_new(9001);
    g_sref = sref;
    XrServer_listen(sref);
    XrServer_free(&sref);

}

