#include "runloop.h"
#include "rl_internal.h"

void Watcher_call_handler(RunloopEventBaseRef this)
{

}
RunloopRef Watcher_get_reactor(RunloopEventBaseRef this)
{
    return this->runloop;
}
int runloop_watcher_base_get_fd(RunloopEventBaseRef this)
{
    return this->fd;
}