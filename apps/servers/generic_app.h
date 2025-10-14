#ifndef H_generic_app_H
#define H_generic_app_H
#include <interfaces/server_app_interface.h>
void* generic_app_new(RunloopRef rl, int connection_fd);
void generic_app_free(void* app_ref);
void generic_app_run(void* app_ref, void(cb)(void* app_ref, void* server_ref, int fd), void* arg);

#endif