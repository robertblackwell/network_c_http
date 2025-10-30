#ifndef H_echo_app_h
#define H_echo_app_h
#include <apps/msg/newline/newline_msg.h>
#include <apps/tcp_msg_stream/msg_stream.h>
#include <apps/server/server_ctx.h>
#define SimpleApp_TAG "SMPAPP"
typedef struct SimpleApp_s {
    RBL_DECLARE_TAG;
    MsgStreamRef msg_stream_ref;
    AppDoneCallback* done_cb;
    void *done_arg;
    RBL_DECLARE_END_TAG;    
} SimpleApp, *SimpleAppRef;
typedef void(SimpleAppDoneCallback)(SimpleAppRef app_ref, void* arg, int error);
typedef void(SimpleAppCallback)(SimpleAppRef app);

SimpleAppRef simple_app_new(RunloopRef rl, int connection_fd);
void simple_app_free(SimpleAppRef app);
void simple_app_run(SimpleAppRef app_ref, AppDoneCallback cb, void* arg);
typedef struct AppInterface
{
    void*(*new)(RunloopRef rl, int fd);
    void(*run)(void* app_ref, void(done_cb)(void* app_ref, void* server, int error), void* server);
    void(*free)(void* app_ref);
} AppInterface, *AppInterfaceRef;
AppInterfaceRef echo_app_interface();

#endif