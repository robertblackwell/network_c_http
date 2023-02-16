
#define CHLOG_ON
#include <http_in_c/async/connection_internal.h>
//static void event_handler(RtorStreamRef stream_ref, uint64_t event);
static void write_epollout(AsyncConnectionRef connection_ref);
static void read_epollin(AsyncConnectionRef connection_ref);

/////////////////////////////////////////////////////////////////////////////////////
// event handler called from the Reactor on receiving an epoll event
///////////////////////////////////////////////////////////////////////////////////////
void async_event_handler(RtorStreamRef stream_ref, uint64_t event)
{
    LOG_FMT("event_handler %lx", event);
    AsyncConnectionRef connection_ref = stream_ref->context;
    CHECK_TAG(AsyncConnection_TAG, connection_ref)
    LOG_FMT("async_handler handler socket: %d", connection_ref->socket);
    if(event & EPOLLOUT) {
        LOG_FMT("EPOLLOUT")
        write_epollout(connection_ref);
    }
    if(event & EPOLLIN) {
        LOG_FMT("EPOLLIN")
        read_epollin(connection_ref);
    }
    if(!((event & EPOLLIN) || (event & EPOLLOUT))){
        LOG_FMT("not EPOLLIN and not EPOLLOUT")
    }
}
static void read_epollin(AsyncConnectionRef connection_ref)
{
    CHECK_TAG(AsyncConnection_TAG, connection_ref)
    LOG_FMT("read_epollin read_state: %s", async_read_state_str(connection_ref->read_state))
    if(connection_ref->read_state == READ_STATE_EAGAINED) {
        connection_ref->read_state = READ_STATE_ACTIVE;
        async_read_start(connection_ref);
    }
}
static void write_epollout(AsyncConnectionRef connection_ref)
{
    CHECK_TAG(AsyncConnection_TAG, connection_ref)
    LOG_FMT("write_epollout")
    if(connection_ref->write_state == WRITE_STATE_EAGAINED) {
        connection_ref->write_state = WRITE_STATE_ACTIVE;
        connection_ref->writeside_posted = true;
        async_post_to_reactor(connection_ref, &async_postable_writer);
    }
}
