#ifndef H_C_HTTP_RUNLOOP_DEFINES_H
#define H_C_HTTP_RUNLOOP_DEFINES_H

/**
 * This section of this file contains macros that control the size of various Runloop relaed data structures.
 */
// max number of event watcher structures per runloop
#define RL_MAX_WATCHERS            1024
// size of the ready list or functor list - max number of callables that can be waiting
// to be run. Depends on RL_MAX_WATCHERS and how many 'green threads' can be running for each watcher.
// for example under epoll a full duplex socket connection would need 2 callables per socket(file descriptor)
// where as a request/response protocol like http would only require 1.
#define RL_GTHREADS_PER_WATCHER    1
#define RL_MAX_RUNLIST             RL_MAX_WATCHERS * RL_GTHREADS_PER_WATCHER
// max size of array used when calling epoll_wait of kevent
#define RL_MAX_EVENTS              RL_MAX_WATCHERS
#if 0
#define runloop_MAX_ITQ            256
#define runloop_MAX_WATCHERS       runloop_MAX_FDS
#define runloop_MAX_EVENTS         runloop_MAX_WATCHERS*2
#define runloop_FUNCTOR_LIST_MAX   runloop_MAX_ITQ
#define runloop_MAX_EPOLL_FDS      runloop_MAX_FDS
#define CBTABLE_MAX                runloop_MAX_FDS
#define runloop_FDTABLE_MAX        runloop_MAX_FDS
#define runloop_READY_LIST_MAX     (2 * runloop_MAX_FDS)
#endif
/**
 * This next section contains flags that enable/disable features
 */
#define RL_EPOLL_EVENTFD_ENABLE 1 // on linux under epoll use eventfd for user_event

#endif