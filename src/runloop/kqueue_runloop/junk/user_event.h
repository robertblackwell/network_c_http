#ifndef C_HTTP_runloop_W_EVENTFD_H
#define C_HTTP_runloop_W_EVENTFD_H
#include <kqueue_runloop/runloop.h>
/** \defgroup eventfd EventFd
 * @{
 * ## Event Fd
 *
 * epoll provides a facility to create a file descriptor that is not attached to any file/pipe/device
 * and to "fire" events on that file descriptor that can be waited for using the epoll call.
 *
 * This RunloopEventFd struct and related functions use the epoll facility to provides a
 * generalized mechanism for creating custom events that can be fired and notified
 * using the standard Linux epoll feature.
 *
 */
struct RunloopEventfd_s;
typedef struct RunloopEventfd_s RunloopEventfd, *RunloopEventfdRef;      // Waiter for epoll event fd event

/**
 * epoll provides a facility to create a file descriptor that is not attached to any file/pipe/device
 * and to "fire" events on that file descriptor that can be waited for using the epoll call.
 * This facility provides a mechanism to create and wait on arbitary event sources.
 */
RunloopEventfdRef runloop_eventfd_new(RunloopRef runloop);
void runloop_eventfd_init(RunloopEventfdRef athis, RunloopRef runloop);
void runloop_eventfd_free(RunloopEventfdRef athis);
void runloop_eventfd_register(RunloopEventfdRef athis);
void runloop_eventfd_change_watch(RunloopEventfdRef athis, PostableFunction postable, void* arg, uint64_t watch_what);
void runloop_eventfd_arm(RunloopEventfdRef athis, PostableFunction postable, void* arg);
void runloop_eventfd_disarm(RunloopEventfdRef athis);
void runloop_eventfd_fire(RunloopEventfdRef athis);
void runloop_eventfd_clear_one_event(RunloopEventfdRef athis);
void runloop_eventfd_clear_all_events(RunloopEventfdRef athis);
void runloop_eventfd_deregister(RunloopEventfdRef athis);
void runloop_eventfd_verify(RunloopEventfdRef r);
RunloopRef runloop_eventfd_get_reactor(RunloopEventfdRef athis);
int runloop_eventfd_get_fd(RunloopEventfdRef this);
//#define runloop_eventfd_get_reactor(p) Watcher_get_reactor((RunloopWatcherBaseRef)p)


/** @} */
#endif //C_HTTP_runloop_W_EVENTFD_H
