#include "runloop_internal.h"
#include <rbl/macros.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

#include <unistd.h>
#define KQ_RUNLOOP_USER_EVENT_SEMAPHORE

/**
 *
 * @param ctx
 * @param fd
 * @param event
 */
static void handler(RunloopEventBaseRef watcher, uint16_t filter, uint16_t flags, void* data)
{
    RunloopSignalRef fdev = (RunloopSignalRef)watcher;
    printf("user event handler entered\n");
    SIGNAL_CHECK_TAG(fdev)
    SIGNAL_CHECK_END_TAG(fdev)
}
void runloop_signal_init(RunloopSignalRef this, RunloopRef runloop)
{
    RBL_ASSERT((this!=NULL), "this is NULL");
    this->type = RUNLOOP_USER_EVENT;
    SIGNAL_SET_TAG(this);
    SIGNAL_SET_END_TAG(this);
    SIGNAL_CHECK_TAG(this)
    SIGNAL_CHECK_END_TAG(this)
    /*
     * The readfd must be NONBLOCK
     */
#ifdef KQ_RUNLOOP_USER_EVENT_TWO_PIPE_TRICK
    RBL_LOG_FMT("two pipe trick enabled")
    int pipefds[2];
    pipe(pipefds);
    this->uevent.read_fd = pipefds[0];
    this->uevent.write_fd = pipefds[1];
#else
    // register_user_event(runloop, this);
    #ifdef KQ_RUNLOOP_USER_EVENT_SEMAPHORE
        RBL_LOG_FMT("two pipe trick disabled semaphore enabled")
        // this->fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC | EFD_SEMAPHORE);
    #else
    RBL_LOG_FMT("two pipe trick disabled semaphore disabled")
        this->fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    #endif
#endif
    this->runloop = runloop;
    this->handler = &handler;
}
RunloopSignalRef runloop_signal_new(RunloopRef runloop)
{
    RunloopSignalRef sig_event = runloop_event_allocate(runloop, sizeof(RunloopSignal));
    runloop_signal_init(sig_event, runloop);
    return sig_event;
}
void runloop_signal_free(RunloopSignalRef sig_event)
{
    SIGNAL_SET_TAG(sig_event);
    SIGNAL_CHECK_TAG(sig_event)
    kqh_signal_event_cancel(sig_event);
    runloop_event_free(sig_event->runloop, sig_event);

}
void runloop_signal_register(RunloopSignalRef rlevent)
{
    SIGNAL_SET_TAG(rlevent);
    SIGNAL_CHECK_TAG(rlevent)
    kqh_user_event_register(rlevent);
    #if 0
    SIGNAL_SET_TAG(athis);
    SIGNAL_CHECK_TAG(athis)

    uint32_t interest = 0L;
    athis->uevent.uevent_postable = NULL;
    athis->uevent.uevent_postable_arg = NULL;
    /**
     * Make sure this call enabled level triggering of events on this fd
     */
    int res = runloop_register(athis->runloop, athis->fd, interest, (RunloopEventBaseRef) (athis));
    assert(res ==0);
    #endif
}
void runloop_signal_change_watch(RunloopSignalRef athis, PostableFunction postable, void* arg, uint64_t watch_what)
{
    #if 0
    SIGNAL_SET_TAG(athis);
    SIGNAL_CHECK_TAG(athis)
    uint32_t interest = watch_what;
    if( postable != NULL) {
        athis->uevent.uevent_postable = postable;
    }
    if (arg != NULL) {
        athis->uevent.uevent_postable_arg = arg;
    }
    int res = runloop_reregister(athis->runloop, athis->fd, interest, (RunloopEventBaseRef) athis);
    assert(res == 0);
    #endif
}
void runloop_signal_deregister(RunloopSignalRef athis)
{
    #if 0
    SIGNAL_SET_TAG(athis);
    SIGNAL_CHECK_TAG(athis)
    int res = runloop_deregister(athis->runloop, athis->fd);
    assert(res == 0);
    #endif
}
void runloop_signal_arm(RunloopSignalRef rlevent, PostableFunction postable, void* arg)
{
    SIGNAL_SET_TAG(rlevent);
    SIGNAL_CHECK_TAG(rlevent)
  if( postable != NULL) {
        rlevent->uevent.uevent_postable = postable;
    }
    if (arg != NULL) {
        rlevent->uevent.uevent_postable_arg = arg;
    }
    kqh_user_event_register(rlevent);
}
void runloop_signal_disarm(RunloopSignalRef rlevent)
{
    SIGNAL_SET_TAG(rlevent);
    SIGNAL_CHECK_TAG(rlevent)
    kqh_user_event_pause(rlevent);
    #if 0
    SIGNAL_SET_TAG(athis);
    SIGNAL_CHECK_TAG(athis)
    int res = runloop_reregister(athis->runloop, athis->fd, 0, (RunloopEventBaseRef) athis);
    #endif
}
void runloop_signal_fire(RunloopSignalRef athis)
{
    SIGNAL_SET_TAG(athis);
    SIGNAL_CHECK_TAG(athis)
#ifdef RUNLOOP_SIGNAL_TWO_PIPE_TRICK
    uint64_t buf = 1;
    write(athis->uevent.write_fd, &buf, sizeof(buf));
#else
    kqh_user_event_trigger(athis, (void*) 1234);
#endif
}
void runloop_signal_clear_one_event(RunloopSignalRef athis)
{
    SIGNAL_SET_TAG(athis);
    SIGNAL_CHECK_TAG(athis)
    uint64_t buf;
    int nread = read(athis->uevent.read_fd, &buf, sizeof(buf));
}
void runloop_signal_clear_all_events(RunloopSignalRef athis)
{
    SIGNAL_SET_TAG(athis);
    SIGNAL_CHECK_TAG(athis)
    uint64_t buf;
    while(1) {
        int nread = read(athis->uevent.read_fd,  &buf, sizeof(buf));
        if (nread == -1) break;
    }
    assert(errno == EAGAIN);
}

RunloopRef runloop_signal_get_runloop(RunloopSignalRef athis)
{
    SIGNAL_SET_TAG(athis);
    SIGNAL_CHECK_TAG(athis)
    return athis->runloop;
}
int runloop_signal_get_fd(RunloopSignalRef athis)
{
    SIGNAL_SET_TAG(athis);
    SIGNAL_CHECK_TAG(athis)
    return athis->uevent.read_fd;
}

void runloop_signal_verify(RunloopSignalRef athis, const char* file, int line_nbr)
{
    SIGNAL_SET_TAG(athis);
    SIGNAL_CHECK_TAG(athis)
}
