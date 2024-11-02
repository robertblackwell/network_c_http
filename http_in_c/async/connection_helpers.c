

#define RBL_LOG_ENABLE
#include <http_in_c/async/connection_internal.h>

//static void event_handler(RunloopStreamRef stream_ref, uint64_t event);
//static void write_epollout(AsyncConnectionRef connection_ref);
//static void read_epollin(AsyncConnectionRef connection_ref);
//
//static void read_start(AsyncConnectionRef connection_ref);
//static void postable_reader(RunloopRef reactor_ref, void* arg);
//static void reader(AsyncConnectionRef connection_ref);
//static llhttp_errno_t on_read_message_complete(http_parser_t* parser_ptr, MessageRef msg);
//static void read_error(AsyncConnectionRef connection_ref, char* msg);
//static void read_eagain(AsyncConnectionRef cref);
//static void read_need_data(AsyncConnectionRef cref);
//
//static void postable_writer(RunloopRef reactor_ref, void* arg);
//static void postable_write_call_cb(RunloopRef reactor_ref, void* arg);
//static void writer(AsyncConnectionRef connection_ref);
//static void write_error(AsyncConnectionRef connection_ref, char* msg);
//
//static void postable_cleanup(RunloopRef reactor, void* cref);
//const char* read_state_str(int state);
//const char* write_state_str(int state);

void async_post_to_reactor(AsyncConnectionRef connection_ref, void(*postable_function)(RunloopRef, void*))
{
    runloop_post(connection_ref->reactor_ref, postable_function, connection_ref);
}

const char* async_read_state_str(int state)
{
    switch(state) {
        case READ_STATE_IDLE:
            return "READ_STATE_IDLE";
        case READ_STATE_ACTIVE:
            return "READ_STATE_ACTIVE";
        case READ_STATE_EAGAINED:
            return "READ_STATE_EAGAINED";
        case READ_STATE_STOP:
            return "READ_STATE_STOP";
        case READ_STATE_POSTED_READER:
            return "READ_STATE_POSTED_READER";
        default:
            RBL_ASSERT(false, "Invalid read state");
    }
}
const char* async_write_state_str(int state)
{
    switch(state) {
        case WRITE_STATE_IDLE:
            return "WRITE_STATE_IDLE";
        case WRITE_STATE_ACTIVE:
            return "WRITE_STATE_ACTIVE";
        case WRITE_STATE_EAGAINED:
            return "WRITE_STATE_EAGAINED";
        case WRITE_STATE_STOP:
            return "WRITE_STATE_STOP";
        default:
            RBL_ASSERT(false, "Invalid read state");
    }
}
const char* async_epoll_event_str(int event)
{
    return "";
}