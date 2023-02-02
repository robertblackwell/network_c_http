#include <c_http/demo_protocol/demo_server.h>
#include <c_http/common/message.h>

#include <stdio.h>
#include <mcheck.h>
#include<signal.h>

DemoServerRef g_sref;
void sig_handler(int signo)
{
    printf("demo_app.c signal handler \n");
    if ((signo == SIGINT) || (signo == SIGABRT)) {
        printf("received SIGINT or SIGABRT\n");
        DemoServer_free(g_sref);
        g_sref = NULL;
        exit(0);
    }
}

int main()
{
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        printf("sync_app.c main signal() failed");
    }
    if (signal(SIGABRT, sig_handler) == SIG_ERR) {
        printf("sync_app.c main signal() failed");
    }
    printf("Hello this is xr-junk main \n");
    DemoServerRef sref = DemoServer_new(9011);
    g_sref = sref;
    DemoServer_listen(sref);
    DemoServer_dispose(&sref);

}

