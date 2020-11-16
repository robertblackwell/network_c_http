#ifndef REACTOR_H
#define REACTOR_H

#include <stdint.h>
#include <time.h>

typedef void (*Callback)(void *arg, int fd, uint32_t events);

typedef struct reactor Reactor, *ReactorRef;

ReactorRef reactor_new(void);

int reactor_destroy(ReactorRef reactor);

int reactor_register(ReactorRef reactor, int fd, uint32_t interest, Callback callback, void *callback_arg);

int reactor_deregister(ReactorRef reactor, int fd);

int reactor_reregister(ReactorRef reactor, int fd, uint32_t interest, Callback callback, void *callback_arg);

int reactor_run(ReactorRef reactor, time_t timeout);

#endif // REACTOR_H
