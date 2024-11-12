#ifndef c_http_queue_watcher_h
#define c_http_queue_watcher_h
#include <time.h>
#include <stdint.h>
#include <http_in_c/runloop/watcher.h>
#include <http_in_c/runloop/reactor.h>
#include <http_in_c/runloop/evfd_queue.h>

/**
 * NOT tested yet - also not used
 *
 * The goal of this object is to provide an interthread synchronization mechaism that
 * can be used qith epoll.
 *
 * So that the mechanism used for listen, read, write and timer can also be used
 * for inter thread communication.
 *
 * One would normally do this with a Condition Variable, but condition variables dont integrate
 * with the epoll_wait mechaism.
 */

#define TYPE WQueue
#define WQueue_TAG "XRLST"
#include <rbl/check_tag.h>
#undef TYPE
#define XR_WQUEUE_DECLARE_TAG DECLARE_TAG(WQueue)
#define XR_WQUEUE_CHECK_TAG(p) CHECK_TAG(WQueue, p)
#define XR_WQUEUE_SET_TAG(p) SET_TAG(WQueue, p)

struct WQueue_s;
typedef struct WQueue_s WQueue, *WQueueRef;
typedef uint64_t XrQueueEvent;

typedef void(XrQueuetWatcherCaller(void* ctx));

struct WQueue_s {
    struct Watcher_s;
    EvfdQueueRef            queue;
    // reactor cb and arg
    QueueEventHandler*      queue_event_handler;
    void*                   queue_event_handler_arg;
};

WQueueRef WQueue_new(ReactorRef runloop, EvfdQueueRef qref);
void WQueue_dispose(WQueueRef this);
void WQueue_register(WQueueRef this, QueueEventHandler cb, void* arg,  uint64_t watch_what);
void WQueue_change_watch(WQueueRef this, QueueEventHandler cb, void* arg, uint64_t watch_what);

void WQueue_deregister(WQueueRef this);



#endif