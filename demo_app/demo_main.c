#include <c_http/demo_protocol/demo_server.h>
#include <c_http/common/message.h>

#include <stdio.h>
#include <mcheck.h>
#include<signal.h>

DemoServerRef g_sref;
int handler_example(MessageRef request, void* wrtr)
{
}
void sig_handler(int signo)
{
    printf("app.c signal handler \n");
    if (signo == SIGINT) {
        printf("received SIGINT\n");
        DemoServer_terminate( g_sref);
    }
}

int main()
{
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        printf("app.c main signal() failed");
    }
    printf("Hello this is xr-junk main \n");
    DemoServerRef sref = DemoServer_new(9011);
    g_sref = sref;
    DemoServer_listen(sref);
    DemoServer_dispose(&sref);

}

