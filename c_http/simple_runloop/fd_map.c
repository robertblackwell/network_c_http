#include <c_http/simple_runloop/rl_internal.h>
#include <assert.h>

bool fd_map[CBTABLE_MAX];
void fd_map_init() {
    for(int i = 0; i < CBTABLE_MAX)
        fd_map[i] = false
}

bool fd_map_at(int j) {
    assert(j >= 0 && j < CBTABLE_MAX);
    return fd_map[j];
}

bool fd_map_set(int j) {
    assert(j >= 0 && j < CBTABLE_MAX);
    return fd_map[j] = true;
}
