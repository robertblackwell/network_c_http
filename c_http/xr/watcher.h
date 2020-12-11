#ifndef c_http_watcher_h
#define c_http_watcher_h
#include <c_http/xr/types.h>
#include <stdint.h>

#define TYPE Watcher
#define Watcher_TAG "XRWCHR"
#include <c_http/check_tag.h>
#undef TYPE
#define XR_WATCHER_DECLARE_TAG DECLARE_TAG(Watcher)


// -fms-extensions is required on compiler for the way Watcher_s is extended "without names"
// to define sub classes
// Read
// https://modelingwithdata.org/arch/00000113.htm
//
//
//
// base struct for watchers
struct Watcher_s {
    XR_WATCHER_DECLARE_TAG;
    WatcherType           type;
    XrReactorRef            runloop;
    int                     fd;
    /**
     * function that knows how to free the specific type of watcher from a general ref
     */
    void(*free)(WatcherRef);
    /**
     * first level handler function - first parameter must be cast to specific type of watcher
    */
    void(*handler)(WatcherRef watcher_ref, int fd, uint64_t event);
};

void Watcher_call_handler(WatcherRef this);

#endif