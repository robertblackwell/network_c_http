#include <echo_app/echo_app.h>
MessageRef process_input_message(MessageRef input_msg);

EchoAppRef echo_app_new(RunloopRef rl, int connection_fd)
{
    EchoAppRef appref = malloc(sizeof(EchoApp));
    echo_app_init(appref, rl, connection_fd);
    return appref;
}
void echo_app_init(EchoAppRef app, RunloopRef rl, int connection_fd)
{
    RBL_SET_TAG(EchoApp_TAG, app)
    RBL_SET_END_TAG(EchoApp_TAG, app)
    app->msg_stream_ref = msg_stream_new(rl, connection_fd);
    // other stuff to come
}

void echo_app_deinit(EchoAppRef app)
{
    msg_stream_deinit(app->msg_stream_ref);
    msg_parser_deinit(app->msgParser_ref);
}
void echo_app_free(EchoAppRef app)
{
    msg_stream_free(app->msg_stream_ref);
    msg_parser_free(app->msgParser_ref);
    free(app);
}
static void msg_read_callback(void* arg, MessageRef msg, int error);
static void msg_write_callback(void* arg, int error);
static void postable_read(RunloopRef rl, void* arg);

void echo_app_run(EchoAppRef app, AppDoneCallback* cb, void* arg)
{
    RunloopRef rl = runloop_stream_get_runloop(app->msg_stream_ref->tcp_stream_ref->rlstream_ref);
    runloop_post(rl, postable_read, app);
}
static void msg_read_callback(void* arg, MessageRef msg, int error)
{
    EchoAppRef app = arg;
    RBL_CHECK_TAG(EchoApp_TAG, app)
    RBL_CHECK_END_TAG(EchoApp_TAG, app)
    if(error != 0) {

    } 
    MessageRef response = process_input_message(msg);
    msg_stream_write(app->msg_stream_ref, response, msg_write_callback, app);
} 
static void msg_write_callback(void* arg, int error)
{
    EchoAppRef app = arg;
    RBL_CHECK_TAG(EchoApp_TAG, app)
    RBL_CHECK_END_TAG(EchoApp_TAG, app)
    if(error != 0) {

    }
    RunloopRef rl = runloop_stream_get_runloop(app->msg_stream_ref->tcp_stream_ref->rlstream_ref);
    runloop_post(rl, postable_read, app);
}
static void postable_read(RunloopRef rl, void* arg)
{
    EchoAppRef app = arg;
    RBL_CHECK_TAG(EchoApp_TAG, app)
    RBL_CHECK_END_TAG(EchoApp_TAG, app)
    msg_stream_read(app->msg_stream_ref, msg_read_callback, app);
}
MessageRef process_input_message(MessageRef input_msg)
{
    return NULL;
}