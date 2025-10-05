#include <kqueue_runloop/runloop.h>
#include <kqueue_runloop/rl_internal.h>
#include <assert.h>

bool fd_map[runloop_MAX_FDS];
void fd_map_init() {
    for(int i = 0; i < runloop_MAX_FDS; i++)
        fd_map[i] = false;
}

bool fd_map_at(int j) {
    assert(j >= 0 && j < runloop_MAX_FDS);
    return fd_map[j];
}

bool fd_map_set(int j) {
    assert(j >= 0 && j < runloop_MAX_FDS);
    return fd_map[j] = true;
}
