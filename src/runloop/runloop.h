#ifndef RUNLOOP_H
#define RUNLOOP_H
    
    #if defined(__linux__) || defined(__APPLE__)
        // #pragma message("We are on a Linux or apple machine")
        #include "runloop_api.h"
    #elif defined(__WIN32)
        #pragma message("On a windows machine")
    #endif
#endif