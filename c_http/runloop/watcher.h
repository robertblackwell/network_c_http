#ifndef c_http_watcher_h
#define c_http_watcher_h
#include <c_http/async/types.h>
#include <stdint.h>

/**
 * Watcher_s is a base "class" for a number of different type specific watchers. 
 * See enum WatcherType in types.h
 * 
 * Each derived class is based on a type specific struct which includes the generic Watcher_s
 * at the start of the type specific struct "without names". EG
 * 
 *   struct WTimer_s {
 *      struct Watcher_s;
 *      
 *      time_t                  expiry_time;
 *      uint64_t                interval;
 *      bool                    repeating;
 *      TimerEventHandler*      timer_handler;
 *      void*                   timer_handler_arg;
 *   };
 *
 * To make this work one needs to compile with -fms-extensions "
 * to define sub classes
 * Read https://modelingwithdata.org/arch/00000113.htm
 * 
 * The downside of this approach is that one needs to cast from Watcher* to WTimer* for example
 * and thats easy to get wrong.
 * 
 * Hence I have implemented a tagging scheme so that every type specific access to a watcher
 * checks the tag to make sure it is accessing the right type of watcher
 * 
*/

/**
 * 
 */
#define TYPE Watcher
#define Watcher_TAG "XRWCHR"
#include <c_http/check_tag.h>
#undef TYPE
#define XR_WATCHER_DECLARE_TAG DECLARE_TAG(Watcher)


struct Watcher_s {
    XR_WATCHER_DECLARE_TAG;
    WatcherType           type;
    XrReactorRef          runloop;
    int                   fd;
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