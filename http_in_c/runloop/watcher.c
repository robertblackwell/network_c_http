#include <http_in_c/runloop/runloop.h>
#include <http_in_c//runloop/rl_internal.h>

void Watcher_call_handler(RunloopWatcherRef this)
{

}
RunloopRef Watcher_get_reactor(RunloopWatcherRef this)
{
    return this->runloop;
}
int Watcher_get_fd(RunloopWatcherRef this)
{
    return this->fd;
}