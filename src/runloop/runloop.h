#ifndef RUNLOOP_H
#define RUNLOOP_H
    
    #ifdef __linux__ 
        #include "epoll_runloop/runloop.h"
    #elif defined(__APPLE__)
        #pragma message("We are on an apple machine")
        #include "kqueue_runloop/runloop.h"
    #elif defined(__WIN32)
        #pragma message("On a windows machine")
    #endif
#endif