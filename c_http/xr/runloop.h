#ifndef c_http_xr_runloop_h
#define c_http_xr_runloop_h

#include <stdint.h>
#include <time.h>

typedef void (*Callback)(void *arg, int fd, uint32_t events);

typedef struct XrRunloop_s XrRunloop, *XrRunloopRef;
typedef struct XrWatcher_s XrWatcher, *XrWatcherRef;


XrRunloopRef XrRunloop_new(void);

int XrRunloop_destroy(XrRunloopRef rl);

int XrRunloop_register(XrRunloopRef rl, int fd, uint32_t interest, XrWatcherRef wref);

int XrRunloop_deregister(XrRunloopRef rl, int fd);

int XrRunloop_reregister(XrRunloopRef rl, int fd, uint32_t interest, XrWatcherRef wref);

int XrRunloop_run(XrRunloopRef rl, time_t timeout);

#endif
