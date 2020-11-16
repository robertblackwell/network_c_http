#ifndef c_http_reactor_timers_h
#define c_http_reactor_timers_h
#include <c_http/list.h>
#include <time.h>
#include <c_http/reactor/cbtable.h>

struct R_Timer_s;
typedef struct R_Timer_s R_Timer, *R_TimerRef;

typedef void(R_TimerCallback(R_TimerRef timer, void* arg, uint64_t event));


struct R_Timer_s {
    int     fd;
    time_t  expiry_time;
    CallbackData cb_data;
};

struct R_TimerList_s {
    int     fd; // the fd being used for timers - hay get removed in future
    int     earliest_expiry;
    ListRef list;
};
typedef struct R_TimerList_s R_TimerList, *R_TimerListRef;

R_TimerRef RTimer_new();
void RTimer_free(R_TimerRef this);

R_TimerListRef RTimerList_new();
void RTimerList_free(R_TimerListRef this);


#endif