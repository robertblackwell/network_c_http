#include <kqueue_runloop/runloop.h>
#include <kqueue_runloop/rl_internal.h>
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
static void handler(RunloopEventRef event_ref, uint64_t event)
{
    RunloopEventRef fdev = (RunloopEventRef)event_ref;
    EVENTFD_CHECK_TAG(fdev)
    EVENTFD_CHECK_END_TAG(fdev)
    uint64_t buf;
    long nread = read(fdev->uevent.write_fd, &buf, sizeof(buf));
    if(nread == sizeof(buf)) {
        fdev->uevent.uevent_postable(fdev->runloop, fdev->uevent.uevent_postable);
    } else {

    }
}
static void anonymous_free(RunloopEventRef p)
{
    RunloopEventRef fdevp = (RunloopEventRef)p;
    EVENTFD_CHECK_TAG(fdevp)
    EVENTFD_CHECK_END_TAG(fdevp)
    runloop_user_event_free(fdevp);
}
void runloop_user_event_init(RunloopEventRef this, RunloopRef runloop)
{
    RBL_ASSERT((this!=NULL), "this is NULL");
    this->type = RUNLOOP_WATCHER_FDEVENT;
    EVENTFD_SET_TAG(this);
    EVENTFD_SET_END_TAG(this);
    EVENTFD_CHECK_TAG(this)
    EVENTFD_CHECK_END_TAG(this)
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
RunloopEventRef runloop_event_new(RunloopRef runloop)
{
    RunloopEventRef this = event_allocator_alloc(runloop->event_allocator);
    runloop_user_event_init(this, runloop);
    return this;
}
void runloop_event_free(RunloopEventRef athis)
{
    EVENTFD_SET_TAG(athis);
    EVENTFD_CHECK_TAG(athis)
    close(athis->uevent.write_fd);
    free((void*)athis);
}
void runloop_event_register(RunloopEventRef athis)
{
    #if 0
    EVENTFD_SET_TAG(athis);
    EVENTFD_CHECK_TAG(athis)

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
    EVENTFD_SET_TAG(athis);
    EVENTFD_CHECK_TAG(athis)
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
    EVENTFD_SET_TAG(athis);
    EVENTFD_CHECK_TAG(athis)
    int res = runloop_deregister(athis->runloop, athis->fd);
    assert(res == 0);
    #endif
}
void runloop_user_event_arm(RunloopEventRef athis, PostableFunction postable, void* arg)
{
    #if 0
    EVENTFD_SET_TAG(athis);
    EVENTFD_CHECK_TAG(athis)
    uint32_t interest = 0;//EPOLLIN | EPOLLERR | EPOLLRDHUP;
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
void runloop_user_event_disarm(RunloopEventRef athis)
{
    #if 0
    EVENTFD_SET_TAG(athis);
    EVENTFD_CHECK_TAG(athis)
    int res = runloop_reregister(athis->runloop, athis->fd, 0, (RunloopWatcherBaseRef) athis);
    #endif
}
void runloop_user_event_fire(RunloopEventRef athis)
{
    EVENTFD_SET_TAG(athis);
    EVENTFD_CHECK_TAG(athis)
#ifdef RUNLOOP_EVENTFD_TWO_PIPE_TRICK
    uint64_t buf = 1;
    write(athis->uevent.write_fd, &buf, sizeof(buf));
#else
    uint64_t buf = 1;
    int x = write(athis->uevent.write_fd, &buf, sizeof(buf));
    assert(x == sizeof(buf));
#endif
}
void runloop_user_event_clear_one_event(RunloopEventRef athis)
{
    EVENTFD_SET_TAG(athis);
    EVENTFD_CHECK_TAG(athis)
    uint64_t buf;
    int nread = read(athis->uevent.read_fd, &buf, sizeof(buf));
}
void runloop_user_event_clear_all_events(RunloopEventRef athis)
{
    EVENTFD_SET_TAG(athis);
    EVENTFD_CHECK_TAG(athis)
    uint64_t buf;
    while(1) {
        int nread = read(athis->uevent.read_fd,  &buf, sizeof(buf));
        if (nread == -1) break;
    }
    assert(errno == EAGAIN);
}

RunloopRef runloop_user_event_get_runloop(RunloopEventRef athis)
{
    EVENTFD_SET_TAG(athis);
    EVENTFD_CHECK_TAG(athis)
    return athis->runloop;
}
int runloop_user_event_get_fd(RunloopEventRef athis)
{
    EVENTFD_SET_TAG(athis);
    EVENTFD_CHECK_TAG(athis)
    return athis->uevent.read_fd;
}

void runloop_user_event_verify(RunloopEventRef athis)
{
    EVENTFD_SET_TAG(athis);
    EVENTFD_CHECK_TAG(athis)
}
