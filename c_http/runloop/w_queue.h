#ifndef c_http_queue_watcher_h
#define c_http_queue_watcher_h
#include <time.h>
#include <stdint.h>
#include <c_http/runloop/watcher.h>
#include <c_http/runloop/reactor.h>
#include <c_http/runloop/evfd_queue.h>

#define TYPE WQueue
#define WQueue_TAG "XRLST"
#include <c_http/check_tag.h>
#undef TYPE
#define XR_QUEUE_DECLARE_TAG DECLARE_TAG(WQueue)
#define XR_QUEUE_CHECK_TAG(p) CHECK_TAG(WQueue, p)
#define XR_QUEUE_SET_TAG(p) SET_TAG(WQueue, p)

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

WQueueRef WQueue_new(XrReactorRef runloop, EvfdQueueRef qref);
void WQueue_dispose(WQueueRef this);
void WQueue_register(WQueueRef this, QueueEventHandler cb, void* arg,  uint64_t watch_what);
void WQueue_change_watch(WQueueRef this, QueueEventHandler cb, void* arg, uint64_t watch_what);

void WQueue_deregister(WQueueRef this);



#endif