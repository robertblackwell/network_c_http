#ifndef c_http_watcher_h
#define c_http_watcher_h
#include <c_http/xr/types.h>
#include <stdint.h>

#define TYPE XrWatcher
#define XrWatcher_TAG "XRWCHR"
#include <c_http/check_tag.h>
#undef TYPE
#define XR_WATCHER_DECLARE_TAG DECLARE_TAG(XrWatcher)
//#define XR_CONN_CHECK_TAG(p) CHECK_TAG(XrWatcher, p)
//#define XR_CONN_SET_TAG(p) SET_TAG(XrWatcher, p)


// -fms-extensions is required on compiler for the way XrWatcher_s is extended "without names"
// to define sub classes
// Read
// https://modelingwithdata.org/arch/00000113.htm
//
//
//
// base struct for watchers
struct XrWatcher_s {
    XR_WATCHER_DECLARE_TAG;
    XrWatcherType           type;
    XrReactorRef            runloop;
    int                     fd;
    /**
     * function that knows how to free the specific type of watcher from a general ref
     */
    void(*free)(XrWatcherRef);
    /**
     * first level handler function - first parameter must be cast to specific type of watcher
    */
    void(*handler)(XrWatcherRef watcher_ref, int fd, uint64_t event);
};

void XrWatcher_call_handler(XrWatcherRef this);

#endif