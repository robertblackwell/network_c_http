#ifndef H_server_app_interface_H
#define H_server_app_interface_H

typedef struct ServerAppInterface {
    void*(*new)(void* rl, int fd);
    void(*run)(void* app_ref, void(done_cb)(void* app_ref, void* server, int error), void* server);
    void(*free)(void* app_ref);
} ServerAppInterface, *ServerAppInterfaceRef;

#endif
