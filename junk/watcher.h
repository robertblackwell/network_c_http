#ifndef c_http_watcher_h
#define c_http_watcher_h
#include <c_http/simple_runloop/reactor.h>
#include <stdint.h>
#if 0
/**
 * 
 * What are watchers
 * 
 * Linux provides three broad types of file descriptors that can be watched using epoll.
 * 
 * -    fd that represents an io device or file against which one can do things like read/write
 *      and in some cases connect listen accept
 * -    eventfd - a special purpose fd so that general purpose events can be triggered and caught 
 *      using epoll
 * -    timerfd - another special purpose fd that can be created and used to provide one-shot or repeating
 *      timers.
 * in addition and only for convenience one more watcher is introduced,
 * -    listener - a watcher where the fd is a socket in non-blocking listen mode. In practice listen sockets 
 *      are not used for reading and writing but only for an accept() call after a listen event occurs.
 *      This just (IMO) makes server code a little bit cleaner.
 * 
 * 
 * Inheritence in C 
 * 
 * Watcher_s, Watcher, WatcgerRef is a base "class" for upon which are built
 * three specific fd watcher classes.
 * 
 * 
 * Those classes are (in the same order as discussed above):
 * -    IoFd_s, IoFd, IoFdRef
 * -    EventFd_s, EventFd, EventFdRef
 * -    TimerFd_s, TimerFd, TimerFdRef
 * -    ListenerFd_s, Listener, ListenerRef
 * 
 * The following is a description of how the inheritence or subclassing has been achived. 
 * See enum WatcherType in types.h
 * 
 * Each derived class is based on a type specific struct which includes the generic Watcher_s
 * at the start of the type specific struct "without names". EG
 * 
 *   struct WTimerFd_s {
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
 * The downside of this approach is that one needs to cast from Watcher* to WTimerFd* for example
 * and thats easy to get wrong.
 * 
 * Further there are times when a Watcher instance must be de-initialized and/or free()ed
 * from a general reference. To achieve this each spcific type must provide a free/dealloc
 * function.
 * 
 * Similarly each specific type must provide a type specific handler function.
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
    ReactorRef          runloop;
    int                   fd;
    /**
     * function that knows how to free the specific sub type of watcher from a general ref.
     * each derived type must provide this function when an instance is created or initializez.
     * In the case of timerfd and event fd watchers must also close the fd
     */
    void(*free)(WatcherRef);
    /**
     * first level handler function
     * each derived type provides thier own type specific handler when an instance is created
     * or initialized and must cast the first parameter to their own specific type of watcher
     * inside the handler.
     * 
     * This handler will be calledd directly from the epoll_wait code inside reactor.c
    */
    void(*handler)(WatcherRef watcher_ref, int fd, uint64_t event);
};
typedef struct Watcher_s Watcher, *WatcherRef;

void Watcher_call_handler(WatcherRef this);

#endif
#endif