
#define CHLOG_ON
#include <http_in_c/async/connection_internal.h>

static void postable_reader(ReactorRef reactor_ref, void* arg);
static void read_new_message(AsyncConnectionRef cref);
static void read_error(AsyncConnectionRef connection_ref, char* msg);
static void read_eagain(AsyncConnectionRef cref);
static void read_need_data(AsyncConnectionRef cref);
static void post_read_start(AsyncConnectionRef cref);
//static void reader(AsyncConnectionRef cref);
static void post_delegate_handle_request(AsyncHandlerRef href, MessageRef request);
static void post_delegate_handle_connection_closed(AsyncHandlerRef href);

void async_read_start(AsyncConnectionRef connection_ref)
{
    CHECK_TAG(AsyncConnection_TAG, connection_ref)
    LOG_FMT("async_read_start socket: %d read_state: %d %s", connection_ref->socket, connection_ref->read_state, async_read_state_str(connection_ref->read_state));
    CHTTP_ASSERT((connection_ref->read_state == READ_STATE_ACTIVE), "invalid state");
    CHTTP_ASSERT((! connection_ref->readside_posted), "read_side posted should be false");
    connection_ref->readside_posted = true;
    rtor_reactor_post(connection_ref->reactor_ref, &postable_reader, connection_ref);
}
static void post_read_start(AsyncConnectionRef cref)
{
    CHECK_TAG(AsyncConnection_TAG, cref);
    CHTTP_ASSERT((! cref->readside_posted), "read_side posted should be false");
    cref->readside_posted = true;
    rtor_reactor_post(cref->reactor_ref, &postable_reader, cref);
}
static void postable_reader(ReactorRef reactor_ref, void* arg)
{
    AsyncConnectionRef connection_ref = arg;
    CHECK_TAG(AsyncConnection_TAG, connection_ref)
    LOG_FMT("postable_reader socket: %d read_state: %d %s", connection_ref->socket, connection_ref->read_state, async_read_state_str(connection_ref->read_state));
    CHTTP_ASSERT((connection_ref->read_state == READ_STATE_ACTIVE), "invalid state");
    CHTTP_ASSERT((connection_ref->readside_posted), "read_side posted should be true");
    connection_ref->readside_posted = false;
    reader(connection_ref);
}
void reader(AsyncConnectionRef connection_ref) {
    CHECK_TAG(AsyncConnection_TAG, connection_ref)
    CHTTP_ASSERT((connection_ref->read_state == READ_STATE_ACTIVE), "invalid state");
    CHTTP_ASSERT((connection_ref->input_message_ptr  == NULL), "input_message_ptr not null - invalid state");
    AsyncHandlerRef href = connection_ref->handler_ref;
    LOG_FMT("reader socket: %d read_state: %d %s", connection_ref->socket, connection_ref->read_state, async_read_state_str(connection_ref->read_state));
    int buffer_size = connection_ref->read_buffer_size;
    if (connection_ref->active_input_buffer_ref == NULL) {
        connection_ref->active_input_buffer_ref = IOBuffer_new_with_capacity(buffer_size);
    } else {
        IOBuffer_reset(connection_ref->active_input_buffer_ref);
    }
    IOBufferRef iob = connection_ref->active_input_buffer_ref;
    void *input_buffer_ptr = IOBuffer_space(iob);
    int input_buffer_length = IOBuffer_space_len(iob);
    int fd = connection_ref->socket;
    int nread = recv(fd, input_buffer_ptr, input_buffer_length, MSG_DONTWAIT);
    if(nread > 0)
        IOBuffer_commit(iob, nread);
    int errno_save = errno;
    LOG_FMT("bytes_available %d errno: %d %s", nread, errno_save, strerror(errno_save));
    int eagain = EAGAIN;
    char* errstr = strerror(errno_save);
    if(nread > 0) {
        LOG_FMT("Before AsyncParser_consume read_state %s", async_read_state_str(connection_ref->read_state));
        llhttp_errno_t ec = http_parser_consume(connection_ref->http_parser_ptr, IOBuffer_data(iob), IOBuffer_data_len(iob));
        LOG_FMT("After AsyncParser_consume returns  message_ptr: %p errno: %d errno_str: %s  read_state %s ",
                connection_ref->input_message_ptr, errno_save, strerror(errno_save), async_read_state_str(connection_ref->read_state));
        if(connection_ref->input_message_ptr != NULL) {
            read_new_message(connection_ref);
            return;
        }
        if(errno_save == EAGAIN) {
            bool x = llhttp_message_needs_eof(connection_ref->http_parser_ptr->m_llhttp_ptr);
            LOG_FMT("test this result message_needs_eof %d", (int)x);
            read_eagain(connection_ref);
            return;
        }
        if(ec == HPE_OK) {
            read_need_data(connection_ref);
        } else {
            read_error(connection_ref, "parser error");
        }
    } else if(nread == 0) {
        LOG_FMT("bytes_available == 0 errno: %d %s", errno_save, strerror(errno_save))
        read_error(connection_ref, "reader zero bytes - peer closed connection");
    } else if (errno_save == eagain) {
        LOG_FMT("bytes_available < 0 eagain errno: %d %s", errno_save, strerror(errno_save))
        read_eagain(connection_ref);
    } else {
        LOG_FMT("bytes_available < 0 io-error errno: %d %s", errno_save, strerror(errno_save))
        read_error(connection_ref, "reader - io error close and move on");
    }
    LOG_FMT("reader return");
}
static void read_new_message(AsyncConnectionRef cref)
{
    CHECK_TAG(AsyncConnection_TAG, cref)
    CHTTP_ASSERT((cref->read_state == READ_STATE_ACTIVE), "invalid state");
    CHTTP_ASSERT((cref->input_message_ptr  != NULL), "input_message_ptr null on new message");
    CHTTP_ASSERT((!cref->readside_posted), "nothing should be posted on readside at this point");
    LOG_FMT("socket: %d", cref->socket)
    AsyncHandlerRef href = cref->handler_ref;
    CHECK_TAG(AsyncHandler_TAG, href);
    MessageRef msg = cref->input_message_ptr;
    cref->input_message_ptr = NULL;
    cref->read_state = READ_STATE_IDLE;
    post_delegate_handle_request(href, msg);
}
static void read_eagain(AsyncConnectionRef cref)
{
    CHECK_TAG(AsyncConnection_TAG, cref)
    CHTTP_ASSERT((cref->read_state == READ_STATE_ACTIVE), "invalid state");
    CHTTP_ASSERT((cref->input_message_ptr  == NULL), "input_message_ptr NOT null should be handled by read_new_message");
    CHTTP_ASSERT((!cref->readside_posted), "nothing should be posted on readside at this point");
    LOG_FMT("read_eagain socket: %d connection_ref->read_state: %s", cref->socket, async_read_state_str(cref->read_state));
    cref->read_state = READ_STATE_EAGAINED;
}
static void read_need_data(AsyncConnectionRef cref)
{
    CHECK_TAG(AsyncConnection_TAG, cref)
    CHTTP_ASSERT((cref->read_state == READ_STATE_ACTIVE), "invalid state");
    CHTTP_ASSERT((cref->input_message_ptr  == NULL), "input_message_ptr NOT null on incomplete read");
    CHTTP_ASSERT((!cref->readside_posted), "nothing should be posted on readside at this point");
    LOG_FMT("read_need_data socket:%d connection_ref->read_state: %s", cref->socket, async_read_state_str(cref->read_state));
//    async_post_to_reactor(cref, &postable_reader);
//    cref->readside_posted = true;
    post_read_start(cref);
}
static void postable_close_connection(ReactorRef reactor_ref, void* arg)
{
    AsyncHandlerRef href = arg;
    CHECK_TAG(AsyncHandler_TAG, href);
    AsyncConnectionRef cref = href->async_connection_ref;
    LOG_FMT("socket: %d", cref->socket)
    href->handle_close_connection(href);
}
static void read_error(AsyncConnectionRef cref, char* msg)
{
    CHECK_TAG(AsyncConnection_TAG, cref)
    CHTTP_ASSERT((cref->read_state == READ_STATE_ACTIVE), "invalid state");
    CHTTP_ASSERT((cref->input_message_ptr  == NULL), "input_message_ptr NOT null should not happen on read error");
    CHTTP_ASSERT((!cref->readside_posted), "nothing should be posted on readside at this point");
    LOG_FMT("socket: %d", cref->socket)
    LOG_FMT("Read_error got an error this is the message: %s  fd: %d", msg, cref->socket);
    AsyncHandlerRef href = cref->handler_ref;
    CHECK_TAG(AsyncHandler_TAG, href);
    cref->read_state = READ_STATE_STOP;
    cref->write_state = WRITE_STATE_STOP;
    rtor_reactor_post(cref->reactor_ref, &postable_close_connection, href);
}

llhttp_errno_t async_on_read_message_complete(http_parser_t* parser_ptr, MessageRef msg)
{
    CHECK_TAG(HTTP_PARSER_TAG, parser_ptr)
    AsyncConnectionRef connection_ref = parser_ptr->handler_context;
    CHECK_TAG(AsyncConnection_TAG, connection_ref)
    AsyncHandlerRef handler_ref = connection_ref->handler_ref;
    CHECK_TAG(AsyncHandler_TAG, handler_ref)
    CHTTP_ASSERT((msg != NULL), "msg should not be NULL");
    CHTTP_ASSERT((connection_ref->read_state == READ_STATE_ACTIVE), "must be read_state ACTIVE");
    LOG_FMT("socket: %d", connection_ref->socket)
    connection_ref->input_message_ptr = msg;
    LOG_FMT("read_message_handler - on_write_cb  read_state: %s", async_read_state_str(connection_ref->read_state));
    return HPE_OK;
}
static void postable_handle_request(ReactorRef reactor_ref, void* arg)
{
    AsyncHandlerRef href = arg;
    CHECK_TAG(AsyncHandler_TAG, href);
    CHECK_TAG(AsyncConnection_TAG, href->async_connection_ref);
    AsyncConnectionRef cref = href->async_connection_ref;
    CHTTP_ASSERT((cref->scratch_request != NULL),"request should not be null");
    MessageRef tmp = cref->scratch_request;
    cref->scratch_request = NULL;
    href->handle_request(href, tmp);
}
static void postable_handle_close_connection(ReactorRef reactor_ref, void* arg)
{
    AsyncHandlerRef href = arg;
    CHECK_TAG(AsyncHandler_TAG, href);
    CHECK_TAG(AsyncConnection_TAG, href->async_connection_ref);
    href->handle_close_connection(href);
}
static void post_delegate_handle_request(AsyncHandlerRef href, MessageRef request)
{
    CHTTP_ASSERT((href->handle_request != NULL), "handle_request should not be null");
    href->async_connection_ref->scratch_request = request;
    rtor_reactor_post(href->async_connection_ref->reactor_ref, postable_handle_request, href);
}
