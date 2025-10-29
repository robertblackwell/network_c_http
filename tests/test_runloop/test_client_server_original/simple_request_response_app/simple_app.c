#include "simple_app.h"
GenericMsgRef process_input_message(GenericMsgRef input_msg);

void simple_app_init(SimpleAppRef app, RunloopRef rl, int connection_fd)
{
    RBL_SET_TAG(SimpleApp_TAG, app)
    RBL_SET_END_TAG(SimpleApp_TAG, app)
    app->msg_stream_ref = msg_stream_new(rl, connection_fd);
    // other stuff to come
}

SimpleAppRef simple_app_new(RunloopRef rl, int connection_fd)
{
    SimpleAppRef appref = malloc(sizeof(SimpleApp));
    simple_app_init(appref, rl, connection_fd);
    return appref;
}
void simple_app_free(SimpleAppRef app)
{
    msg_stream_free(app->msg_stream_ref);
    free(app);
}
AppInterface simple_app_interface_variable;
AppInterfaceRef simple_app_interface()
{
    AppInterfaceRef ai = & simple_app_interface_variable;
    ai->new = (void*(*)(RunloopRef, int))simple_app_new;
    ai->run = (void(*)(void*, void(*)(void*, void*, int), void*))(simple_app_run);
    ai->free = (void(*)(void*))(simple_app_free);
    return ai;
}
static void msg_read_callback(void* arg, GenericMsgRef msg, int error);
static void msg_write_callback(void* arg, int error);
static void postable_read(RunloopRef rl, void* arg);
static void invoke_done_callback(SimpleAppRef app, int error);

void simple_app_run(SimpleAppRef app, AppDoneCallback* cb, void* arg)
{
    app->done_cb = cb;
    app->done_arg = arg;
    RunloopRef rl = runloop_stream_get_runloop(app->msg_stream_ref->tcp_stream_ref->rlstream_ref);
    runloop_post(rl, postable_read, app);
}
static void msg_read_callback(void* arg, GenericMsgRef msg, int error)
{
    SimpleAppRef app = arg;
    RBL_CHECK_TAG(SimpleApp_TAG, app)
    RBL_CHECK_END_TAG(SimpleApp_TAG, app)
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
    SimpleAppRef app = arg;
    RBL_CHECK_TAG(SimpleApp_TAG, app)
    RBL_CHECK_END_TAG(SimpleApp_TAG, app)
    if(error != 0) {

    }
    RunloopRef rl = runloop_stream_get_runloop(app->msg_stream_ref->tcp_stream_ref->rlstream_ref);
    runloop_post(rl, postable_read, app);
}
static void postable_read(RunloopRef rl, void* arg)
{
    SimpleAppRef app = arg;
    RBL_CHECK_TAG(SimpleApp_TAG, app)
    RBL_CHECK_END_TAG(SimpleApp_TAG, app)
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
static void invoke_done_callback(SimpleAppRef app, int error)
{
    AppDoneCallback* cb = app->done_cb;
    void* arg = app->done_arg;
    app->done_cb = NULL;
    app->done_arg = NULL;
    cb(app, arg, error);
}