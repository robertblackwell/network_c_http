#include <c_http/simple_runloop/runloop.h>
#include <c_http//simple_runloop/rl_internal.h>

void Watcher_call_handler(WatcherRef this)
{

}
ReactorRef Watcher_get_reactor(WatcherRef this)
{
    return this->runloop;
}
int Watcher_get_fd(WatcherRef this)
{
    return this->fd;
}