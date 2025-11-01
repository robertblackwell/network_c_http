#ifndef H_server_app_interface_H
#define H_server_app_interface_H
typedef struct ServerAppInterface {
    void*(*new)(void* rl, int fd);
    void(*run)(void* app_ref, void(done_cb)(void* app_ref, void* server, int error), void* server);
    void(*free)(void* app_ref);
} ServerAppInterface, *ServerAppInterfaceRef;

void* generic_app_new(void* runloop, int connection_fd);
void generic_app_free(void* app_ref);
void generic_app_run(void* app_ref, void(cb)(void* app_ref, void* server_ref, int fd), void* arg);


#endif
