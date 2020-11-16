#include <c_http/reactor/timers.h>
#include <sys/timerfd.h>
#include <c_http/list.h>
#include <stdlib.h>

R_TimerRef RTimer_new()
{
}
void RTimer_free(R_TimerRef this)
{
}
static void dealloc(void** t)
{
    RTimer_free((R_TimerRef)t);
}
void RTimerList_init(R_TimerListRef this)
{
    this->fd = timerfd_create(CLOCK_REALTIME, 0);
    this->list = List_new(&dealloc);
}
R_TimerListRef RTimerList_new()
{
    R_TimerListRef tmp = malloc(sizeof(R_TimerList));
    RTimerList_init(tmp);
    return tmp;
}
void RTimerList_free(R_TimerListRef this)
{
}
int RTimerList_earliest_expiry_time(R_TimerListRef this)
{
}
