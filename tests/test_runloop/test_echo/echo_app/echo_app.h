#ifndef H_echo_app_h
#define H_echo_app_h
#include <msg/msg_stream.h>
#include <server/server_ctx.h>
#define EchoApp_TAG "ECHTAG"
typedef struct EchoApp_s {
    RBL_DECLARE_TAG;
    MsgStreamRef msg_stream_ref;
    AppDoneCallback* done_cb;
    void *done_arg;
    RBL_DECLARE_END_TAG;    
} EchoApp, *EchoAppRef;
typedef void(EchoAppDoneCallback)(EchoAppRef app_ref, void* arg, int error);
typedef void(EchoAppCallback)(EchoAppRef app);

EchoAppRef echo_app_new(RunloopRef rl, int connection_fd);
void echo_app_init(EchoAppRef app, RunloopRef rl, int connection_fd);

void echo_app_deinit(EchoAppRef app);
void echo_app_free(EchoAppRef app);
void echo_app_run(EchoAppRef app_ref, AppDoneCallback cb, void* arg);


#endif