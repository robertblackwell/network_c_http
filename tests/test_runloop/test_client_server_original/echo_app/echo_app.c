#include "echo_app.h"
GenericMsgRef process_input_message(GenericMsgRef input_msg);

void echo_app_init(EchoAppRef app, RunloopRef rl, int connection_fd)
{
    RBL_SET_TAG(EchoApp_TAG, app)
    RBL_SET_END_TAG(EchoApp_TAG, app)
    app->msg_stream_ref = msg_stream_new(rl, connection_fd);
    // other stuff to come
}

EchoAppRef echo_app_new(RunloopRef rl, int connection_fd)
{
    EchoAppRef appref = malloc(sizeof(EchoApp));
    echo_app_init(appref, rl, connection_fd);
    return appref;
}
void echo_app_free(EchoAppRef app)
{
    msg_stream_free(app->msg_stream_ref);
    free(app);
}
AppInterface echo_app_interface_variable;
AppInterfaceRef echo_app_interface()
{
    AppInterfaceRef ai = & echo_app_interface_variable;
    ai->new = (void*(*)(RunloopRef, int))echo_app_new;
    ai->run = (void(*)(void*, void(*)(void*, void*, int), void*))(echo_app_run);
    ai->free = (void(*)(void*))(echo_app_free);
    return ai;
}
static void msg_read_callback(void* arg, GenericMsgRef msg, int error);
static void msg_write_callback(void* arg, int error);
static void postable_read(RunloopRef rl, void* arg);
static void invoke_done_callback(EchoAppRef app, int error);

void echo_app_run(EchoAppRef app, AppDoneCallback* cb, void* arg)
{
    app->done_cb = cb;
    app->done_arg = arg;
    RunloopRef rl = runloop_stream_get_runloop(app->msg_stream_ref->tcp_stream_ref->rlstream_ref);
    runloop_post(rl, postable_read, app);
}
static void msg_read_callback(void* arg, GenericMsgRef msg, int error)
{
    EchoAppRef app = arg;
    RBL_CHECK_TAG(EchoApp_TAG, app)
    RBL_CHECK_END_TAG(EchoApp_TAG, app)
    if(error != 0) {
        printf("msg_read_callback error %d  %s\n ", error, strerror(error));
        invoke_done_callback(app, error);
    } else {
        assert(msg != NULL);
        GenericMsgRef response = process_input_message(msg);
        generic_msg_free(msg);
        msg_stream_write(app->msg_stream_ref, response, msg_write_callback, app);
    }
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

GenericMsgRef process_input_message(GenericMsgRef input_msg)
{
    GenericMsgRef response = generic_msg_new();
    char* p = IOBuffer_data(generic_msg_get_content(input_msg));

    char* response_buf_ptr;
    asprintf(&response_buf_ptr, "ServerResponse:[%s]", p);
    IOBufferRef tmp = IOBuffer_from_cstring(response_buf_ptr);
    free(response_buf_ptr);

    generic_msg_set_content(response, tmp);
    return response;
}
static void invoke_done_callback(EchoAppRef app, int error)
{
    AppDoneCallback* cb = app->done_cb;
    void* arg = app->done_arg;
    app->done_cb = NULL;
    app->done_arg = NULL;
    cb(app, arg, error);
}