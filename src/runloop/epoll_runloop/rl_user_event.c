#include <src/runloop/epoll_runloop/event_table.h>
#include "runloop_internal.h"
#include <rbl/macros.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include "epoll_helper.h"
#define RUNLOOP_USER_EVENT_SEMAPHORE

/**
 *
 * @param ctx
 * @param fd
 * @param event
 */
static void handler(RunloopWatcherBaseRef fdevent_ref, uint64_t event)
{
    RunloopUserEventRef fdev = (RunloopUserEventRef)fdevent_ref;
    USER_EVENT_CHECK_TAG(fdev)
    USER_EVENT_CHECK_END_TAG(fdev)
    uint64_t buf;
    while(1) {
        ssize_t nread = read(fdev->fd, &buf, sizeof(buf));
        int errno_saved = errno;
//        printf("uevent handler nread: %ld  buf: %lu\n", nread, buf);
        if (nread == sizeof(buf)) {
            fdev->fdevent_postable(fdev->runloop, fdev->fdevent_postable_arg);
        } else if((nread < 0) && (errno_saved == EAGAIN)){
            return;
        } else {
            assert(0);
        }
    }
}
// static void anonymous_free(RunloopWatcherBaseRef p)
// {
//     RunloopUserEventRef fdevp = (RunloopUserEventRef)p;
//     USER_EVENT_CHECK_TAG(fdevp)
//     USER_EVENT_CHECK_END_TAG(fdevp)
//     runloop_user_event_free(fdevp);
// }
void runloop_user_event_init(RunloopUserEventRef uevent, RunloopRef runloop)
{
    RBL_ASSERT((uevent!=NULL), "this is NULL");
    uevent->type = RUNLOOP_WATCHER_FDEVENT;
    USER_EVENT_SET_TAG(uevent);
    USER_EVENT_SET_END_TAG(uevent);
    USER_EVENT_CHECK_TAG(uevent)
    USER_EVENT_CHECK_END_TAG(uevent)
    /*
     * The readfd must be NONBLOCK
     */
#ifdef RUNLOOP_USER_EVENT_TWO_PIPE_TRICK
    RBL_LOG_FMT("two pipe trick enabled")
    int pipefds[2];
    pipe(pipefds);
    uevent->fd = pipefds[0];
    uevent->write_fd = pipefds[1];
#else
    #ifdef RUNLOOP_USER_EVENT_SEMAPHORE
        RBL_LOG_FMT("two pipe trick disabled semaphore enabled")
    uevent->fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC | EFD_SEMAPHORE);
    #else
    RBL_LOG_FMT("two pipe trick disabled semaphore disabled")
        uevent->fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    #endif
#endif
    uevent->runloop = runloop;
    // this->free = &anonymous_free;
    uevent->handler = &handler;
    runloop_user_event_register(uevent);
}
RunloopUserEventRef runloop_user_event_new(RunloopRef runloop)
{
    RunloopUserEventRef this = rl_event_allocate(runloop, sizeof(RunloopUserEvent));
    runloop_user_event_init(this, runloop);
    return this;
}
void runloop_user_event_free(RunloopUserEventRef athis)
{
    USER_EVENT_SET_TAG(athis);
    USER_EVENT_CHECK_TAG(athis)
    close(athis->fd);
    rl_event_free(athis->runloop, athis);
}
void runloop_user_event_register(RunloopUserEventRef athis)
{
    USER_EVENT_SET_TAG(athis);
    USER_EVENT_CHECK_TAG(athis)

    uint32_t interest = 0L;
    athis->fdevent_postable = NULL;
    athis->fdevent_postable_arg = NULL;
    eph_add(athis->runloop->epoll_fd, athis->fd, interest, athis);
}
void runloop_user_event_deregister(RunloopUserEventRef athis)
{
    USER_EVENT_SET_TAG(athis);
    USER_EVENT_CHECK_TAG(athis)
    eph_del(athis->runloop->epoll_fd, athis->fd, (uint32_t)0, athis);
//    int res = runloop_deregister(athis->runloop, athis->fd);
//    assert(res == 0);
}
void runloop_user_event_arm(RunloopUserEventRef athis, PostableFunction postable, void* arg)
{
    USER_EVENT_SET_TAG(athis);
    USER_EVENT_CHECK_TAG(athis)
    uint32_t interest = eph_interest_read(false);// | EPOLLERR | EPOLLRDHUP;
    if( postable != NULL) {
        athis->fdevent_postable = postable;
    }
    if (arg != NULL) {
        athis->fdevent_postable_arg = arg;
    }
    eph_mod(athis->runloop->epoll_fd, athis->fd, interest, athis);
//    int res = runloop_reregister(athis->runloop, athis->fd, interest, (RunloopWatcherBaseRef) athis);
//    assert(res == 0);
}
void runloop_user_event_disarm(RunloopUserEventRef athis)
{
    USER_EVENT_SET_TAG(athis);
    USER_EVENT_CHECK_TAG(athis)
    eph_mod(athis->runloop->epoll_fd, athis->fd, (uint32_t)0, athis);
//    int res = runloop_reregister(athis->runloop, athis->fd, 0, (RunloopWatcherBaseRef) athis);
}
void runloop_user_event_fire(RunloopUserEventRef uev)
{
    printf("runloop_user_event_fire %p\n", uev);
    USER_EVENT_SET_TAG(uev);
    USER_EVENT_CHECK_TAG(uev)
#ifdef RUNLOOP_USER_EVENT_TWO_PIPE_TRICK
    uint64_t buf = 1;
    write(athis->write_fd, &buf, sizeof(buf));
#else
    uint64_t buf = 1;
    ssize_t x = write(uev->fd, &buf, sizeof(buf));
    assert(x == sizeof(buf));
#endif
}
void runloop_user_event_clear_one_event(RunloopUserEventRef athis)
{
    USER_EVENT_SET_TAG(athis);
    USER_EVENT_CHECK_TAG(athis)
    uint64_t buf;
    int nread = read(athis->fd, &buf, sizeof(buf));
}
void runloop_user_event_clear_all_events(RunloopUserEventRef athis)
{
    USER_EVENT_SET_TAG(athis);
    USER_EVENT_CHECK_TAG(athis)
    uint64_t buf;
    while(1) {
        int nread = read(athis->fd,  &buf, sizeof(buf));
        if (nread == -1) break;
    }
    assert(errno == EAGAIN);
}

RunloopRef runloop_user_event_get_runloop(RunloopUserEventRef athis)
{
    USER_EVENT_SET_TAG(athis);
    USER_EVENT_CHECK_TAG(athis)
    return athis->runloop;
}
int runloop_user_event_get_fd(RunloopUserEventRef athis)
{
    USER_EVENT_SET_TAG(athis);
    USER_EVENT_CHECK_TAG(athis)
    return athis->fd;
}

void runloop_user_event_verify(RunloopUserEventRef athis)
{
    USER_EVENT_SET_TAG(athis);
    USER_EVENT_CHECK_TAG(athis)
}
