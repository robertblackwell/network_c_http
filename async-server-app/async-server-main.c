#include <http_in_c/async/async.h>

#include <stdio.h>
#include <mcheck.h>
#include<signal.h>

AsyncServerRef g_sref;
void sig_handler(int signo)
{
    printf("demo_app.c signal handler \n");
    if ((signo == SIGINT) || (signo == SIGABRT)) {
        printf("received SIGINT or SIGABRT\n");
        AsyncServer_free(g_sref);
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
    AsyncServerRef sref = AsyncServer_new(9001);
    g_sref = sref;
    AsyncServer_listen(sref);
    AsyncServer_dispose(&sref);

}

