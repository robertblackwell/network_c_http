

#define CHLOG_ON
#include <http_in_c/async/connection_internal.h>

AsyncConnectionRef async_connection_new(
        int socket,
        ReactorRef reactor_ref,
        AsyncHandlerRef handler_ref
)
{
    AsyncConnectionRef this = malloc(sizeof(AsyncConnection));
    async_connection_init(this, socket, reactor_ref, handler_ref);
    return this;
}
void async_connection_init(
        AsyncConnectionRef this,
        int socket,
        ReactorRef reactor_ref,
        AsyncHandlerRef handler_ref
)
{
    SET_TAG(AsyncConnection_TAG, this)
    CHECK_TAG(AsyncConnection_TAG, this)
    LOG_FMT("AsyncConnection socket: %d", socket)
    this->reactor_ref = reactor_ref;
    this->handler_ref = handler_ref;
    this->socket_stream_ref = rtor_stream_new(reactor_ref, socket);
    this->socket_stream_ref->context = this;
    this->active_input_buffer_ref = NULL;
    this->active_output_buffer_ref = NULL;
    this->read_state = READ_STATE_IDLE;
    this->write_state = WRITE_STATE_IDLE;
    this->readside_posted = false;
    this->writeside_posted = false;
    this->cleanup_done_flag = false;
    ParserOnMessageCompleteHandler h = async_on_read_message_complete;
    this->http_parser_ptr = http_parser_new(h, this);
    rtor_stream_register(this->socket_stream_ref);
    this->socket_stream_ref->both_arg = this;
    rtor_stream_arm_both(this->socket_stream_ref, &async_event_handler, this);
    this->read_buffer_size = 1000000;
}
void async_connection_destroy(AsyncConnectionRef this)
{
    CHECK_TAG(AsyncConnection_TAG, this)
    int fd = this->socket_stream_ref->fd;
    this->socket = -1;
    LOG_FMT("async_connection_free close socket: %d", fd)
    SET_TAG("xxxxxxx", this) // corrupt the tag
    close(fd);
    rtor_stream_free(this->socket_stream_ref);
    this->socket_stream_ref = NULL;
    http_parser_dispose(&(this->http_parser_ptr));
    if(this->active_output_buffer_ref) {
        IOBuffer_dispose(&(this->active_output_buffer_ref));
    }
    if(this->active_input_buffer_ref) {
        IOBuffer_dispose(&(this->active_input_buffer_ref));
    }
    INVALIDATE_TAG(this)
    // INVALIDATE_STRUCT(this, AsyncConnection)
}
void async_connection_free(AsyncConnectionRef this)
{
    CHECK_TAG(AsyncConnection_TAG, this)
    async_connection_destroy(this);
    free(this);
}
void async_connection_read(AsyncConnectionRef connection_ref) //, void(*on_read_message_cb)(void* href, MessageRef, int status))
{
    CHECK_TAG(AsyncConnection_TAG, connection_ref)
    LOG_FMT("async_connection_read read_state: %d %s", connection_ref->read_state, read_state_str(connection_ref->read_state));
//    CHTTP_ASSERT((connection_ref->read_state != READ_STATE_ACTIVE), "a read is already active");
    if(connection_ref->read_state == READ_STATE_IDLE) {
        connection_ref->read_state = READ_STATE_ACTIVE;
        async_read_start(connection_ref);
    } else if(connection_ref->read_state == READ_STATE_EAGAINED) {
//        connection_ref->on_read_message_cb = on_read_message_cb;
    } else if(connection_ref->read_state == READ_STATE_STOP) {
        connection_ref->handler_ref->handle_reader_stopped(connection_ref->handler_ref); //, NULL, AsyncConnectionErrCode_is_closed);
    }
}
void async_connection_write(AsyncConnectionRef connection_ref, MessageRef response_ref)
{
    CHECK_TAG(AsyncConnection_TAG, connection_ref)
    CHTTP_ASSERT((response_ref != NULL), "got NULL instead of a response_ref");
    CHTTP_ASSERT((connection_ref->write_state == WRITE_STATE_IDLE), "a write is already active");
    LOG_FMT("response_ref %p", response_ref)
    connection_ref->active_output_buffer_ref = Message_serialize(response_ref);
    if(connection_ref->write_state == WRITE_STATE_STOP) {
        connection_ref->handler_ref->handle_write_failed(connection_ref->handler_ref);
        return;
    }
    async_post_to_reactor(connection_ref, &async_postable_writer);
}
