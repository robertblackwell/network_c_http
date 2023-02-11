

#define CHLOG_ON
#include <http_in_c/async/connection_internal.h>

//static void event_handler(RtorStreamRef stream_ref, uint64_t event);
//static void write_epollout(AsyncConnectionRef connection_ref);
//static void read_epollin(AsyncConnectionRef connection_ref);
//
//static void read_start(AsyncConnectionRef connection_ref);
//static void postable_reader(ReactorRef reactor_ref, void* arg);
//static void reader(AsyncConnectionRef connection_ref);
//static llhttp_errno_t on_read_message_complete(http_parser_t* parser_ptr, MessageRef msg);
//static void read_error(AsyncConnectionRef connection_ref, char* msg);
//static void read_eagain(AsyncConnectionRef cref);
//static void read_need_data(AsyncConnectionRef cref);
//
//static void postable_writer(ReactorRef reactor_ref, void* arg);
void postable_write_call_cb(ReactorRef reactor_ref, void* arg);
static void writer(AsyncConnectionRef connection_ref);
static void write_error(AsyncConnectionRef connection_ref, char* msg);
//
//static void postable_cleanup(ReactorRef reactor, void* cref);
//const char* read_state_str(int state);
//const char* write_state_str(int state);

/**
 * Utility function that wraps all rtor_reactor_post() calls so this module can
 * keep track of outstanding pending function calls
 */

/////////////////////////////////////////////////////////////////////////////////////
// write sequence - sequence of functions called during write operation
////////////////////////////////////////////////////////////////////////////////////
void async_postable_writer(ReactorRef reactor_ref, void* arg)
{
    AsyncConnectionRef connection_ref = arg;
    CHECK_TAG(AsyncConnection_TAG, connection_ref)
    if(connection_ref->write_state == WRITE_STATE_STOP) {
        IOBuffer_dispose(&(connection_ref->active_output_buffer_ref));
        connection_ref->handler_ref->handle_write_failed(connection_ref->handler_ref);
        return;
    }
    CHTTP_ASSERT((connection_ref->active_output_buffer_ref != NULL), "post_write_handler");
    CHTTP_ASSERT((connection_ref->write_state == WRITE_STATE_IDLE), "cannot call write while write state is not idle");
    writer(connection_ref);
}

static void writer(AsyncConnectionRef connection_ref)
{
    CHECK_TAG(AsyncConnection_TAG, connection_ref)
    CHTTP_ASSERT((connection_ref->active_output_buffer_ref != NULL), "writer");
    IOBufferRef iob = connection_ref->active_output_buffer_ref;

    long wrc = send(connection_ref->socket_stream_ref->fd, IOBuffer_data(iob), IOBuffer_data_len(iob), MSG_DONTWAIT);
    int errno_saved = errno;
    if(wrc > 0) {
        IOBuffer_consume(iob, wrc);
        if(IOBuffer_data_len(iob) == 0) {
            // write is complete
            connection_ref->write_state = WRITE_STATE_IDLE;
            IOBuffer_dispose(&(connection_ref->active_output_buffer_ref));
            connection_ref->active_output_buffer_ref = NULL;
            async_post_to_reactor(connection_ref, &async_postable_write_call_cb);
        } else {
            // write not complete schedule another try
            printf("writer errno_saved %d %s\n", errno_saved, strerror(errno_saved));
            connection_ref->write_state = WRITE_STATE_ACTIVE;
            async_post_to_reactor(connection_ref, &async_postable_writer);
        }
    } else if (wrc == 0) {
        write_error(connection_ref, "think the fd is closed by other end");
    } else if ((wrc == -1) && (errno == EAGAIN)) {
        connection_ref->write_state = WRITE_STATE_EAGAINED;
        return;
    } else if(wrc == -1) {
        write_error(connection_ref, "think this was an io error");
    }
}
void async_postable_write_call_cb(ReactorRef reactor_ref, void* arg)
{
    AsyncConnectionRef connection_ref = arg;
    CHECK_TAG(AsyncConnection_TAG, connection_ref)
    AsyncHandlerRef href = connection_ref->handler_ref;
    href->handle_write_done(href);
}
static void write_error(AsyncConnectionRef connection_ref, char* msg)
{
    CHECK_TAG(AsyncConnection_TAG, connection_ref)
    LOG_FMT("Write_error got an error this is the message: %s  fd: %d", msg, connection_ref->socket_stream_ref->fd);
    connection_ref->write_state = WRITE_STATE_STOP;
    connection_ref->handler_ref->handle_write_failed(connection_ref->handler_ref);
    async_post_to_reactor(connection_ref, &async_postable_cleanup);
}
