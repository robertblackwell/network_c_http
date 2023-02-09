#include <http_in_c/runloop/rl_internal.h>
#include <assert.h>

bool fd_map[RTOR_MAX_FDS];
void fd_map_init() {
    for(int i = 0; i < RTOR_MAX_FDS; i++)
        fd_map[i] = false;
}

bool fd_map_at(int j) {
    assert(j >= 0 && j < RTOR_MAX_FDS);
    return fd_map[j];
}

bool fd_map_set(int j) {
    assert(j >= 0 && j < RTOR_MAX_FDS);
    return fd_map[j] = true;
}
