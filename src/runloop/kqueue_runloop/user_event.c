#include "runloop_internal.h"
#include <rbl/macros.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

#include <unistd.h>
#define KQ_RUNLOOP_USER_EVENT_SEMAPHORE

static void handler(RunloopEventRef watcher, uint16_t filter, uint16_t flags)
{
    RunloopEventRef fdev = (RunloopEventRef)watcher;
    printf("user event handler entered\n");
    USER_EVENT_CHECK_TAG(fdev)
    USER_EVENT_CHECK_END_TAG(fdev)
    uint64_t buf;
    watcher->uevent.uevent_postable(watcher->runloop, watcher->uevent.uevent_postable_arg);
}
static void anonymous_free(RunloopEventRef p)
{
    RunloopEventRef fdevp = (RunloopEventRef)p;
    USER_EVENT_CHECK_TAG(fdevp)
    USER_EVENT_CHECK_END_TAG(fdevp)
    runloop_user_event_free(fdevp);
}
void runloop_user_event_init(RunloopEventRef this, RunloopRef runloop)
{
    RBL_ASSERT((this!=NULL), "this is NULL");
    this->type = RUNLOOP_WATCHER_UEVENT;
    USER_EVENT_SET_TAG(this);
    USER_EVENT_SET_END_TAG(this);
    USER_EVENT_CHECK_TAG(this)
    USER_EVENT_CHECK_END_TAG(this)
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
    this->free = &anonymous_free;
    this->handler = &handler;
}
RunloopEventRef runloop_user_event_new(RunloopRef runloop)
{
    RunloopEventRef this = event_table_get_entry(runloop->event_table);
    runloop_user_event_init(this, runloop);
    return this;
}
void runloop_user_event_free(RunloopEventRef athis)
{
    USER_EVENT_SET_TAG(athis);
    USER_EVENT_CHECK_TAG(athis)
    event_table_release_entry(athis->runloop->event_table, athis);
    // close(athis->uevent.write_fd);
    // free((void*)athis);
}
void runloop_user_event_register(RunloopEventRef rlevent)
{
    USER_EVENT_SET_TAG(rlevent);
    USER_EVENT_CHECK_TAG(rlevent)
    kqh_user_event_register(rlevent);
    #if 0
    USER_EVENT_SET_TAG(athis);
    USER_EVENT_CHECK_TAG(athis)

    uint32_t interest = 0L;
    athis->uevent.uevent_postable = NULL;
    athis->uevent.uevent_postable_arg = NULL;
    /**
     * Make sure this call enabled level triggering of events on this fd
     */
    int res = runloop_register(athis->runloop, athis->fd, interest, (RunloopWatcherBaseRef) (athis));
    assert(res ==0);
    #endif
}
void runloop_user_event_change_watch(RunloopEventRef athis, PostableFunction postable, void* arg, uint64_t watch_what)
{
    #if 0
    USER_EVENT_SET_TAG(athis);
    USER_EVENT_CHECK_TAG(athis)
    uint32_t interest = watch_what;
    if( postable != NULL) {
        athis->uevent.uevent_postable = postable;
    }
    if (arg != NULL) {
        athis->uevent.uevent_postable_arg = arg;
    }
    int res = runloop_reregister(athis->runloop, athis->fd, interest, (RunloopWatcherBaseRef) athis);
    assert(res == 0);
    #endif
}
void runloop_user_event_deregister(RunloopEventRef athis)
{
    #if 0
    USER_EVENT_SET_TAG(athis);
    USER_EVENT_CHECK_TAG(athis)
    int res = runloop_deregister(athis->runloop, athis->fd);
    assert(res == 0);
    #endif
}
void runloop_user_event_arm(RunloopEventRef rlevent, PostableFunction postable, void* arg)
{
    USER_EVENT_SET_TAG(rlevent);
    USER_EVENT_CHECK_TAG(rlevent)
  if( postable != NULL) {
        rlevent->uevent.uevent_postable = postable;
    }
    if (arg != NULL) {
        rlevent->uevent.uevent_postable_arg = arg;
    }
    kqh_user_event_register(rlevent);
}
void runloop_user_event_disarm(RunloopEventRef rlevent)
{
    USER_EVENT_SET_TAG(rlevent);
    USER_EVENT_CHECK_TAG(rlevent)
    kqh_user_event_pause(rlevent);
}
void runloop_user_event_fire(RunloopEventRef athis)
{
    USER_EVENT_SET_TAG(athis);
    USER_EVENT_CHECK_TAG(athis)
#ifdef RUNLOOP_USER_EVENT_TWO_PIPE_TRICK
    uint64_t buf = 1;
    write(athis->uevent.write_fd, &buf, sizeof(buf));
#else
    kqh_user_event_trigger(athis, (void*) 1234);
#endif
}
void runloop_user_event_clear_one_event(RunloopEventRef athis)
{
    USER_EVENT_SET_TAG(athis);
    USER_EVENT_CHECK_TAG(athis)
    uint64_t buf;
    assert(0);
}
void runloop_user_event_clear_all_events(RunloopEventRef athis)
{
    USER_EVENT_SET_TAG(athis);
    USER_EVENT_CHECK_TAG(athis)
    uint64_t buf;
    assert(0);
    assert(errno == EAGAIN);
}

RunloopRef runloop_user_event_get_runloop(RunloopEventRef athis)
{
    USER_EVENT_SET_TAG(athis);
    USER_EVENT_CHECK_TAG(athis)
    return athis->runloop;
}
int runloop_user_event_get_fd(RunloopEventRef athis)
{
    USER_EVENT_SET_TAG(athis);
    USER_EVENT_CHECK_TAG(athis)
    return -1;//athis->uevent.read_fd;
}

void runloop_user_event_verify(RunloopEventRef athis)
{
    USER_EVENT_SET_TAG(athis);
    USER_EVENT_CHECK_TAG(athis)
}
