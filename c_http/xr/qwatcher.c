#include <c_http/xr/qwatcher.h>

XrQueueWatcherRef Xrqw_new(XrRunloopRef rl)
{
    return NULL;
}
void Xrqw__free(XrQueueWatcherRef this)
{
}
static void dealloc(void** t)
{
    Xrqw__free((XrQueueWatcherRef )t);
}
//void RTimerList_init(R_TimerListRef this)
//{
//    this->fd = timerfd_create(CLOCK_REALTIME, 0);
//    this->list = List_new(&dealloc);
//}
//R_TimerListRef RTimerList_new()
//{
//    R_TimerListRef tmp = malloc(sizeof(R_TimerList));
//    RTimerList_init(tmp);
//    return tmp;
//}
//void RTimerList_free(R_TimerListRef this)
//{
//}
//int RTimerList_earliest_expiry_time(R_TimerListRef this)
//{
//}
