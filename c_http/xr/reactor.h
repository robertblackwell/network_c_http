#ifndef c_http_xr_runloop_h
#define c_http_xr_runloop_h

#include <stdint.h>
#include <time.h>
#include <c_http/xr/watcher.h>

#define XR_REACTOR_TAG "XRRTOR"
#define XR_REACTOR_TAG_LENGTH 8
#define XR_REACTOR_DECLARE_TAG char tag[XR_REACTOR_TAG_LENGTH]
#define XR_REACTOR_CHECK_TAG(p) \
do { \
    assert(strcmp((p)->tag, XR_REACTOR_TAG) == 0); \
} while(0);

#define XR_REACTOR_SET_TAG(p) \
do { \
    sprintf((p)->tag, "%s", XR_REACTOR_TAG); \
} while(0);


typedef void (*Callback)(void *arg, int fd, uint32_t events);
typedef void (*WatcherCallback)(XrWatcherRef wref, void* arg, uint64_t events);
typedef struct XrReactor_s XrReactor, *XrReactorRef;
typedef struct XrWatcher_s XrWatcher, *XrWatcherRef;


XrReactorRef XrReactor_new(void);

void XrReactor_free(XrReactorRef rtor_ref);

int XrReactor_register(XrReactorRef rtor_ref, int fd, uint32_t interest, XrWatcherRef wref);

int XrReactor_deregister(XrReactorRef rtor_ref, int fd);

int XrReactor_reregister(XrReactorRef rtor_ref, int fd, uint32_t interest, XrWatcherRef wref);

int XrReactor_run(XrReactorRef rtor_ref, time_t timeout);

int XrReactor_post(XrReactorRef rtor_ref, PostableFunction cb, void* arg);

#endif
