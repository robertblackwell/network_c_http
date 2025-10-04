
#define RBL_LOG_ENABLE 1
#define RBL_LOG_ALLOW_GLOBAL 1
#include <stdio.h>
#include <pthread.h>
             /* See feature_test_macros(7) */
#include <fcntl.h>
#include <stdint.h>
#include <sys/epoll.h>
#include <math.h>
#include <rbl/logger.h>
#include <rbl/unittest.h>
#include <src/common/utils.h>
#include <src/runloop/runloop.h>
//#include <src/runloop/rl_internal.h>

#include "./timer_helpers.c"
#include "./timer_single_repeating.c"
#include "./timer_multiple_repeating.c"
void* threadfn(void* arg)
{
    TestCtx* ctx = (TestCtx*)arg;
    if(ctx->selector == 1)
        test_timer_single_repeating();
    else
        test_timer_multiple_repeating();
    return NULL;
}
int make_multiple_threads(int selector)
{
    printf("test_timer_multithread\n");
    int nbr_threads = 5;
    TestCtx* contexts[nbr_threads];
    for(int ix = 0; ix < 5; ix++) {
        contexts[ix] = TestCtx_new(0, 10, 100);
        contexts[ix]->selector = selector;
        int r = pthread_create(&(contexts[ix]->thread), NULL, &threadfn, (void*)&(contexts[ix]));

    }
    for(int ix = 0; ix < 5; ix++) {
        pthread_join(contexts[ix]->thread, NULL);
    }
    for(int ix = 0; ix < 5; ix++) {
        free(contexts[ix]);
    }
    return 0;
}
int test_multiple_threads_single_repeaters() {
    make_multiple_threads(1);
    return 0;
}
int test_multiple_threads_multiple_repeaters() {
    make_multiple_threads(2);
    return 0;
}
int main()
{
    printf("Testing multithreading wth runloop\n");
    UT_ADD(test_multiple_threads_single_repeaters);
    UT_ADD(test_multiple_threads_multiple_repeaters);
    int rc = UT_RUN();
    return rc;
}
