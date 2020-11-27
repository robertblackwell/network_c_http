#ifndef c_http_xr_runloop_h
#define c_http_xr_runloop_h

#include <stdint.h>
#include <time.h>
#include <c_http/xr/watcher.h>

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

int XrReactor_post(XrReactorRef rtor_ref, XrWatcherRef watch, WatcherCallback cb, void* arg);

#endif
