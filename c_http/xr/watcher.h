#ifndef c_http_watcher_h
#define c_http_watcher_h
#include <c_http/xr/types.h>
#include <stdint.h>

// -fms-extensions is required on compiler for the way XrWatcher_s is extended "without names"
// to define sub classes
// Read
// https://modelingwithdata.org/arch/00000113.htm
//
//
//
// base struct for watchers
struct XrWatcher_s {
    XrWatcherType           type;
    XrRunloopRef            runloop;
    int                     fd;
    /**
     * function that knows how to free the specific type of watcher from a general ref
     */
    void(*free)(XrWatcherRef);
    /**
     * handler function for specific type of watcher
    */
    void(*handler)(void* ctx, int fd, uint64_t event);
};

void XrWatcher_call_handler(XrWatcherRef this);

#endif