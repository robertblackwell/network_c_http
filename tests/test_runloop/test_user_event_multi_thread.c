#include <stdio.h>
             /* See feature_test_macros(7) */
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <rbl/logger.h>
#include <rbl/unittest.h>
#include <rbl/check_tag.h>
#include <rbl/macros.h>
#include <src/common/utils.h>
#include <src/runloop/runloop.h>

#define TestCtx_TYPE "Tcxtx"
typedef struct SenderCtx_s  {
    RBL_DECLARE_TAG;
    int                 ident;
    int                 callback1_counter;
    int                 counter;
    int                 max_count;
    RunloopUserEventRef user_event_ref;
    int                 fdevent_counter;
    RBL_DECLARE_END_TAG;
} SenderCtx;
SenderCtx* TestCtx_new(RunloopRef rl, int id, RunloopTimerRef timer_ref, RunloopUserEventRef eventfd_ref, int counter_init, int counter_max);
SenderCtx* SenderCtx_new(int id, RunloopUserEventRef user_event_ref,int counter_init, int counter_max)
{
    SenderCtx* tmp = malloc(sizeof(SenderCtx));
    RBL_SET_TAG(TestCtx_TYPE, tmp)
    RBL_SET_END_TAG(TestCtx_TYPE, tmp)
    tmp->ident = id;
    tmp->counter = counter_init;
    tmp->callback1_counter = 0;
    tmp->max_count = counter_max;
    tmp->fdevent_counter = 0;
    tmp->user_event_ref = user_event_ref;
    return tmp;
}
typedef struct RecvCtx_s
{
    RunloopUserEventRef uevent;
    int counter;
} RecvCtx;
RecvCtx* recv_ctx_new(RunloopUserEventRef uevent)
{
    RecvCtx* tmp = malloc(sizeof(RecvCtx));
    tmp->counter = 0;
    tmp->uevent = uevent;
    return tmp;
}
int test_user_event_multi_thread();
int main()
{
    UT_ADD(test_user_event_multi_thread);
    int rc = UT_RUN();
    return rc;
}
void user_event_postable(RunloopRef rl, void* arg)
{
    RecvCtx* ctx_p = arg;
    RunloopUserEventRef uevent = ctx_p->uevent;
    ctx_p->counter++;
    printf("user_event_postable counter: %d \n", ctx_p->counter);
    runloop_user_event_arm(uevent, user_event_postable, arg);
    RBL_LOG_FMT("arg: %p counter %d", ctx_p, ctx_p->counter);
}

void* sender_thread_func(void* arg)
{
    sleep(1);
    SenderCtx* ctx_p = arg;
    for (int k = 0; k < ctx_p->max_count; k++) {
        printf("sender trigger k: %d ctx_p: %p\n", k, ctx_p);
        #ifdef APPLE_FLAG
        runloop_user_event_fire(ctx_p->user_event_ref, arg);
        #elif defined(LINUX_FLAG)
        printf("about to fire user event \n");
        RunloopUserEventRef uev = ctx_p->user_event_ref;
        runloop_user_event_fire(uev);
        #endif
        //
        // This sleep call must be long enough to prevent multiple triggers being amalgamated into
        // a single event by the kqueue implementation
        //
        sleep(1);
    }
    return NULL;
}
int test_user_event_multi_thread()
{
    int nbr_sender_threads = 1;
    pthread_t sender_threads[nbr_sender_threads];
    SenderCtx* sender_ctx[nbr_sender_threads];

    RunloopRef runloop_ref = runloop_new();
    RecvCtx* recv_ctx = (RecvCtx*)malloc(sizeof(RecvCtx));
    recv_ctx->uevent = runloop_user_event_new(runloop_ref);
    // runloop_user_event_register(uevent);
    runloop_user_event_arm(recv_ctx->uevent, user_event_postable, recv_ctx);
    for (int i = 0; i < nbr_sender_threads; i++) {
        sender_ctx[i] = SenderCtx_new(1, recv_ctx->uevent, 0, 5);
        pthread_create(&sender_threads[i], NULL, sender_thread_func, sender_ctx[i]);
    }
    runloop_run(runloop_ref, 3000);
    int total_count = 0;
    for (int i = 0; i < nbr_sender_threads; i++) {
        total_count += sender_ctx[i]->max_count;
        free(sender_ctx[i]);
    }
    printf("total count: %d recv count: %d\n", total_count, recv_ctx->counter);
#if defined(APPLE_FLAG)
    UT_TRUE(total_count >= recv_ctx->counter);
#elif defined(LINUX_FLAG)
    UT_TRUE(total_count == recv_ctx->counter);
#endif
    return 0;
}
