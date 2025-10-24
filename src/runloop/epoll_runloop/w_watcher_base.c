#include "runloop_internal.h"

void Watcher_call_handler(RunloopWatcherBaseRef this)
{

}
RunloopRef Watcher_get_reactor(RunloopWatcherBaseRef this)
{
    return this->runloop;
}
int runloop_watcher_base_get_fd(RunloopWatcherBaseRef this)
{
    return this->fd;
}