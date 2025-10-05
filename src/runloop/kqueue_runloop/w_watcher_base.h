#ifndef C_HTTP_kq_runloop_W_WATCHER_BASE_H
#define C_HTTP_kq_runloop_W_WATCHER_BASE_H

#include "runloop.h"

RunloopRef runloop_watcher_base_get_runloop(RunloopWatcherBaseRef athis);
int        runloop_watcher_base_get_fd(RunloopWatcherBaseRef this);

#endif //C_HTTP_kq_runloop_W_WATCHER_BASE_H
