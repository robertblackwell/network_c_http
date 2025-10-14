#ifndef H_echo_app_h
#define H_echo_app_h
#include <msg/msg_stream.h>
#include <interfaces/server_app_interface.h>
#include <server/server_ctx.h>
#define EchoApp_TAG "ECHTAG"
typedef struct EchoApp_s {
    RBL_DECLARE_TAG;
    MsgStreamRef msg_stream_ref;
    AppDoneCallback* done_cb;
    void *done_arg;
    RBL_DECLARE_END_TAG;    
} EchoApp, *EchoAppRef;
typedef void(EchoAppDoneCallback)(void* app_ref, void* arg, int error);
typedef void(EchoAppCallback)(EchoAppRef app);

EchoAppRef echo_app_new(RunloopRef rl, int connection_fd);
void echo_app_init(EchoAppRef app, RunloopRef rl, int connection_fd);

void echo_app_deinit(EchoAppRef app);
void echo_app_free(EchoAppRef app);
void echo_app_run(EchoAppRef app_ref, AppDoneCallback cb, void* arg);

ServerAppInterfaceRef echo_app_get_server_app_interface();

/**
 * The generic version of the 3 function interface - it allows link time polymorphism
 */
// void* generic_app_new(void* runloop, int connection_fd);
// void generic_app_free(void* app_ref);
// void generic_app_run(void* app_ref, void(cb)(void* app_ref, void* server_ref, int fd), void* arg);

typedef struct AppInterface
{
    void*(*new)(RunloopRef rl, int fd);
    void(*run)(void* app_ref, void(done_cb)(void* app_ref, void* server, int error), void* server);
    void(*free)(void* app_ref);
} AppInterface, *AppInterfaceRef;
AppInterfaceRef echo_app_interface();

#endif