#ifndef C_HTTP_runloop_W_LISTENER_H
#define C_HTTP_runloop_W_LISTENER_H
#include "runloop.h"
/** \defgroup listener RunloopListener
 * @{
 * ## RunloopListener
 *
 * RunloopListener is a special kind of RunloopWatcherBase that is intended spcifically for situations
 * where a server of sometype has called listen() on a socket and is waiting for notification that
 * an accept() call will succeed because a client has connected.
 * a new connection.
 *
 */
struct RunloopListener_s;
typedef struct RunloopListener_s RunloopListener, * RunloopListenerRef;   // Special event for socket listen()

typedef void(*AcceptCallback)(void* arg, int accepted_fd, int errno);


RunloopListenerRef runloop_listener_new(RunloopRef runloop, int fd);
void runloop_listener_free(RunloopListenerRef athis);

void runloop_listener_init(RunloopListenerRef athis, RunloopRef runloop, int fd);
void runloop_listener_deinit(RunloopListenerRef athis);

void runloop_listener_register(RunloopListenerRef athis, PostableFunction postable, void* postable_arg);
void runloop_listener_deregister(RunloopListenerRef athis);
void runloop_listener_arm(RunloopListenerRef athis, PostableFunction postable, void* postable_arg);
void runloop_listener_disarm(RunloopListenerRef athis);
void runloop_listener_verify(RunloopListenerRef r);
RunloopRef runloop_listener_get_runloop(RunloopListenerRef athis);
int runloop_listener_get_fd(RunloopListenerRef athis);
/** @} */
#endif //C_HTTP_runloop_W_LISTENER_H
