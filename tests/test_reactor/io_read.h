#ifndef c_http_tests_reactor_io_read_h
#define c_http_tests_reactor_io_read_h

#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include <sys/epoll.h>
#include <math.h>
#include <c_http/utils.h>
#include <c_http/xr/reactor.h>
#include <c_http/xr/watcher.h>
#include <c_http/xr/timer_watcher.h>
#include <c_http/xr/socket_watcher.h>

typedef struct ReadCtx_s {
    int                 ctx_tag;
    char*               id;
    int                 readfd;
    XrSocketWatcherRef  swatcher;
} ReadCtx;

typedef struct Reader_s {
    int     rd_tag;
    int     count;
    ReadCtx ctx_table[10];
} Reader;

void Reader_init(Reader* this);
Reader* Reader_new();
void Reader_free(Reader* this);
void Reader_add_fd(Reader* this, int fd);

void rd_callback(XrSocketWatcherRef watch, void* arg, uint64_t event);
void* reader_thread_func(void* arg);
#endif