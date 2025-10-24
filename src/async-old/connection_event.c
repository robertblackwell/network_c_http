#include <src/async/connection_internal.h>
//static void event_handler(RunloopStreamRef stream_ref, uint64_t event);

/////////////////////////////////////////////////////////////////////////////////////
// event handler called from the Runloop on receiving an epoll event
///////////////////////////////////////////////////////////////////////////////////////
#if 0
void async_event_handler(RunloopStreamRef stream_ref, uint64_t event)
{
    RBL_LOG_FMT("event_handler %lx", event);
    AsyncConnectionRef connection_ref = stream_ref->context;
    RBL_CHECK_TAG(AsyncConnection_TAG, connection_ref)
    RBL_LOG_FMT("async_handler handler socket: %d", connection_ref->socket);
    if(event & EPOLLOUT) {
        RBL_LOG_FMT("EPOLLOUT")
        write_epollout(connection_ref);
    }
    if(event & EPOLLIN) {
        RBL_LOG_FMT("EPOLLIN")
        read_epollin(connection_ref);
    }
    if(!((event & EPOLLIN) || (event & EPOLLOUT))){
        RBL_LOG_FMT("not EPOLLIN and not EPOLLOUT")
    }
}
#endif
void read_epollin(RunloopRef rl, void* connection_ref_arg)
{
    AsyncConnectionRef  connection_ref = connection_ref_arg;
    RBL_CHECK_TAG(AsyncConnection_TAG, connection_ref)
    RBL_LOG_FMT("read_epollin read_state: %s", async_read_state_str(connection_ref->read_state))
    if(connection_ref->read_state == READ_STATE_EAGAINED) {
        connection_ref->read_state = READ_STATE_ACTIVE;
        async_read_start(connection_ref);
    }
}
void write_epollout(RunloopRef rl, void* connection_ref_arg)
{
    AsyncConnectionRef  connection_ref = connection_ref_arg;
    RBL_CHECK_TAG(AsyncConnection_TAG, connection_ref)
    RBL_LOG_FMT("write_epollout")
    if(connection_ref->write_state == WRITE_STATE_EAGAINED) {
        connection_ref->write_state = WRITE_STATE_ACTIVE;
        connection_ref->writeside_posted = true;
        async_post_to_reactor(connection_ref, &async_postable_writer);
    }
}
