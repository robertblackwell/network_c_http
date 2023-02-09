#include <http_in_c/runloop/runloop.h>
#include <http_in_c//runloop/rl_internal.h>

void Watcher_call_handler(RtorWatcherRef this)
{

}
ReactorRef Watcher_get_reactor(RtorWatcherRef this)
{
    return this->runloop;
}
int Watcher_get_fd(RtorWatcherRef this)
{
    return this->fd;
}