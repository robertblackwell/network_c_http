#include "runloop_internal.h"
#include <rbl/macros.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

#include <unistd.h>
#define KQ_USER_EVENT_DUP_FD
#undef KQ_USER_EVENT_TWO_PIPE_TRICK

static void handler(RunloopUserEventRef watcher, uint16_t filter, uint16_t flags, void* data)
{
    RunloopUserEventRef fdev = (RunloopUserEventRef)watcher;
    USER_EVENT_CHECK_TAG(fdev)
    USER_EVENT_CHECK_END_TAG(fdev)
#ifdef KQ_USER_EVENT_TWO_PIPE_TRICK
    uint64_t buf;
    long nread = read(fdev->uevent.read_fd, &buf, sizeof(buf));
    if (nread == sizeof(buf)) {
        fdev->uevent.uevent_cb(fdev->runloop, fdev->uevent.uevent_cb_arg);
    }
#else
    printf("user event handler entered filter %x flags: %x data: %p\n", filter, flags, data);
    fdev->uevent.uevent_data = data;
    uint64_t buf;
    fdev->uevent.uevent_cb(watcher->runloop, watcher->uevent.uevent_cb_arg);
#endif
}
void runloop_user_event_init(RunloopUserEventRef user_event, RunloopRef runloop)
{
    RBL_ASSERT((user_event!=NULL), "user_event is NULL");
    user_event->type = RUNLOOP_WATCHER_UEVENT;
    USER_EVENT_SET_TAG(user_event);
    USER_EVENT_SET_END_TAG(user_event);
    USER_EVENT_CHECK_TAG(user_event)
    USER_EVENT_CHECK_END_TAG(user_event)
#ifdef KQ_USER_EVENT_TWO_PIPE_TRICK
    RBL_LOG_FMT("two pipe trick enabled")
    int pipefds[2];
    pipe(pipefds);
    user_event->uevent.read_fd = pipefds[0];
    user_event->uevent.write_fd = pipefds[1];
#else
    // register_user_event(runloop, user_event);
    #ifdef KQ_USER_EVENT_DUP_FD
        RBL_LOG_FMT("two pipe trick disabled kqueue - dup fd")
        user_event->uevent.dup_fd = dup(runloop->kqueue_fd);
    #else
    RBL_LOG_FMT("two pipe trick disabled dup_fd disabled")
        user_event->fd = -1;
    #endif
#endif
    user_event->runloop = runloop;
    user_event->handler = &handler;
    // kqh_user_event_pause(user_event);
}
RunloopUserEventRef runloop_user_event_new(RunloopRef runloop)
{
    RunloopUserEventRef user_event = event_table_get_entry(runloop->event_table);
    runloop_user_event_init(user_event, runloop);
    return user_event;
}
void runloop_user_event_free(RunloopUserEventRef user_event)
{
    USER_EVENT_CHECK_TAG(user_event);
    USER_EVENT_CHECK_END_TAG(user_event)
    kqh_user_event_cancel(user_event);
    event_table_release_entry(user_event->runloop->event_table, user_event);
}
void runloop_user_event_register(RunloopUserEventRef user_event)
{
    USER_EVENT_CHECK_TAG(user_event);
    USER_EVENT_CHECK_END_TAG(user_event)
}
void runloop_user_event_deregister(RunloopUserEventRef user_event)
{
    USER_EVENT_CHECK_TAG(user_event);
    USER_EVENT_CHECK_END_TAG(user_event)
    kqh_user_event_cancel(user_event);
}
void runloop_user_event_arm(RunloopUserEventRef rlevent, UserEventCallback cb, void* cb_arg)
{
    USER_EVENT_CHECK_TAG(rlevent)
    USER_EVENT_CHECK_END_TAG(rlevent);
    if( cb != NULL) {
        rlevent->uevent.uevent_cb = cb;
    }
    if (cb_arg != NULL) {
        rlevent->uevent.uevent_cb_arg = cb_arg;
    }
    kqh_user_event_arm(rlevent);
}
void runloop_user_event_disarm(RunloopUserEventRef rlevent)
{
    USER_EVENT_CHECK_TAG(rlevent)
    USER_EVENT_CHECK_END_TAG(rlevent);
    kqh_user_event_pause(rlevent);
}
void runloop_user_event_fire(RunloopUserEventRef user_event, void* data)
{
    USER_EVENT_CHECK_TAG(user_event)
    USER_EVENT_CHECK_END_TAG(user_event);
#ifdef KQ_USER_EVENT_TWO_PIPE_TRICK
    uint64_t buf = 1;
    write(user_event->uevent.write_fd, &buf, sizeof(buf));
#else
    kqh_user_event_trigger(user_event, (void*) data);
#endif
}

RunloopRef runloop_user_event_get_runloop(RunloopUserEventRef user_event)
{
    USER_EVENT_CHECK_TAG(user_event)
    USER_EVENT_CHECK_END_TAG(user_event);
    return user_event->runloop;
}
void runloop_user_event_verify(RunloopUserEventRef user_event)
{
    USER_EVENT_CHECK_TAG(user_event)
    USER_EVENT_CHECK_END_TAG(user_event);
}
