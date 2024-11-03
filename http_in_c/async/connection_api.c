

#define CHLOG_ON
#include <http_in_c/async/connection_internal.h>
static void async_connection_close(AsyncConnectionRef cref);

AsyncConnectionRef async_connection_new(
        int socket,
        RunloopRef reactor_ref,
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
        RunloopRef reactor_ref,
        AsyncHandlerRef handler_ref
)
{
    RBL_SET_TAG(AsyncConnection_TAG, this)
    RBL_CHECK_TAG(AsyncConnection_TAG, this)
    RBL_LOG_FMT("AsyncConnection socket: %d", socket)
    this->reactor_ref = reactor_ref;
    this->handler_ref = handler_ref;
    this->socket = socket;
    this->socket_stream_ref = runloop_stream_new(reactor_ref, socket);
    this->socket_stream_ref->context = this;
    this->socket_stream_ref->both_arg = this;
    this->active_input_buffer_ref = NULL;
    this->active_output_buffer_ref = NULL;
    ParserOnMessageCompleteHandler h = async_on_read_message_complete;
    this->http_parser_ptr = http_parser_new(h, this);
    this->input_message_ptr = NULL;
    this->output_message_ptr = NULL;
    this->read_state = READ_STATE_IDLE;
    this->write_state = WRITE_STATE_IDLE;
    this->read_buffer_size = 1000000;
    this->readside_posted = false;
    this->writeside_posted = false;
    this->cleanup_done_flag = false;
    runloop_stream_register(this->socket_stream_ref);
    runloop_stream_arm_both(this->socket_stream_ref, &async_event_handler, this);
}
void async_connection_destroy(AsyncConnectionRef this)
{
    RBL_CHECK_TAG(AsyncConnection_TAG, this)
    int fd = this->socket_stream_ref->fd;
    runloop_stream_deregister(this->socket_stream_ref);
    runloop_stream_free(this->socket_stream_ref);
    this->socket_stream_ref = NULL;
    RBL_LOG_FMT("async_connection_free close socket: %d", fd)
    if(this->socket > 0) {
        async_connection_close(this);
    }
    http_parser_dispose(&(this->http_parser_ptr));
    if(this->active_output_buffer_ref) {
        IOBuffer_dispose(&(this->active_output_buffer_ref));
    }
    if(this->active_input_buffer_ref) {
        IOBuffer_dispose(&(this->active_input_buffer_ref));
    }
    RBL_SET_TAG("xxxxxxx", this) // corrupt the tag
    RBL_INVALIDATE_TAG(this)
    // RBL_INVALIDATE_STRUCT(this, AsyncConnection)
}
void async_connection_close(AsyncConnectionRef cref)
{
    RBL_CHECK_TAG(AsyncConnection_TAG, cref)
    RBL_ASSERT((cref->socket > 0), "socket should be positive");

    close(cref->socket);
    cref->socket = -1;
}
void async_connection_free(AsyncConnectionRef this)
{
    RBL_CHECK_TAG(AsyncConnection_TAG, this)
    async_connection_destroy(this);
    free(this);
}
void async_connection_read(AsyncConnectionRef connection_ref) //, void(*on_read_message_cb)(void* href, MessageRef, int status))
{
    RBL_CHECK_TAG(AsyncConnection_TAG, connection_ref)
    RBL_ASSERT((connection_ref->read_state == READ_STATE_IDLE), "can only call async_connection_read once");
    RBL_ASSERT((connection_ref->input_message_ptr == NULL), "already a message waiting");
    RBL_LOG_FMT("href: %p socket: %d read state: %s readside_posted: %d", connection_ref->handler_ref, connection_ref->socket,
                async_read_state_str(connection_ref->read_state), (int)connection_ref->readside_posted)
    connection_ref->read_state = READ_STATE_ACTIVE;
    if(!connection_ref->readside_posted) {
        reader(connection_ref);
    }
//    async_read_start(connection_ref);
}
void async_connection_write(AsyncConnectionRef connection_ref, MessageRef response_ref)
{
    RBL_CHECK_TAG(AsyncConnection_TAG, connection_ref)
    RBL_ASSERT((response_ref != NULL), "got NULL instead of a response_ref");
    RBL_ASSERT((connection_ref->write_state == WRITE_STATE_IDLE), "a write is already active");
    RBL_LOG_FMT("response_ref %p", response_ref)
    connection_ref->active_output_buffer_ref = Message_serialize(response_ref);
    connection_ref->write_state = WRITE_STATE_ACTIVE;
    async_post_to_reactor(connection_ref, &async_postable_writer);
}
