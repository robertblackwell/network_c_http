#ifndef H_echo_app_h
#define H_echo_app_h
#include <msg/msg_selection_header.h>
#include <async_msg_stream/msg_stream.h>
#include <server/server_ctx.h>
#ifdef __cplusplus
extern "C" {
#endif

#define EchoApp_TAG "ECHTAG"
typedef void(AppDoneCallback)(void* app_ref, void* arg, int error);
typedef struct EchoApp_s {
    RBL_DECLARE_TAG;
    MsgStreamRef msg_stream_ref;
    AppDoneCallback* done_cb;
    void *done_arg;
    RBL_DECLARE_END_TAG;    
} EchoApp, *EchoAppRef;

EchoAppRef echo_app_new(RunloopRef rl, int connection_fd);
void echo_app_init(EchoAppRef app, RunloopRef rl, int connection_fd);

void echo_app_deinit(EchoAppRef app);
void echo_app_free(EchoAppRef app);
void echo_app_run(EchoAppRef app_ref, AppDoneCallback cb, void* arg);

#define APP_NEW(rl, fd) echo_app_new(rl, fd)
#define APP_FREE(app_ref) echo_app_free(app_ref)
#define APP_RUN(app_ref, cb, arg) echo_app_run(app_ref, cb, arg)
#ifdef __cplusplus
}
#endif

#endif