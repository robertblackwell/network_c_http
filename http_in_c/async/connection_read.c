
#define CHLOG_ON
#include <http_in_c/async/connection_internal.h>

//static void event_handler(RtorStreamRef stream_ref, uint64_t event);
//static void write_epollout(AsyncConnectionRef connection_ref);
//static void read_epollin(AsyncConnectionRef connection_ref);
//
//static void read_start(AsyncConnectionRef connection_ref);
static void postable_reader(ReactorRef reactor_ref, void* arg);
static void reader(AsyncConnectionRef connection_ref);
//static llhttp_errno_t on_read_message_complete(http_parser_t* parser_ptr, MessageRef msg);
static void read_error(AsyncConnectionRef connection_ref, char* msg);
static void read_eagain(AsyncConnectionRef cref);
static void read_need_data(AsyncConnectionRef cref);
//
//static void postable_writer(ReactorRef reactor_ref, void* arg);
//static void postable_write_call_cb(ReactorRef reactor_ref, void* arg);
//static void writer(AsyncConnectionRef connection_ref);
//static void write_error(AsyncConnectionRef connection_ref, char* msg);
//
//static void postable_cleanup(ReactorRef reactor, void* cref);
//const char* read_state_str(int state);
//const char* write_state_str(int state);

/**
 * Utility function that wraps all rtor_reactor_post() calls so this module can
 * keep track of outstanding pending function calls
 */
//static void post_to_reactor(AsyncConnectionRef connection_ref, void(*postable_function)(ReactorRef, void*));


void async_read_start(AsyncConnectionRef connection_ref)
{
    CHECK_TAG(AsyncConnection_TAG, connection_ref)
    LOGFMT("async_read_start read_state: %d %s", connection_ref->read_state, async_read_state_str(connection_ref->read_state));
    connection_ref->readside_posted = true;
    if(connection_ref->read_state == READ_STATE_STOP) {
        printf("async_read_start XXXXXX \n");
    }
    connection_ref->read_state = READ_STATE_POSTED_READER;
    async_post_to_reactor(connection_ref, &postable_reader);
}
static void postable_reader(ReactorRef reactor_ref, void* arg)
{
    AsyncConnectionRef connection_ref = arg;
    LOGFMT("postable_reader read_state: %d %s", connection_ref->read_state, async_read_state_str(connection_ref->read_state));
    if(connection_ref->read_state == READ_STATE_STOP) {
        connection_ref->handler_ref->handle_reader_stopped(connection_ref->handler_ref);//, NULL, AsyncConnectionErrCode_is_closed);
        return;
    }
    reader(connection_ref);
}
static void reader(AsyncConnectionRef connection_ref) {
    CHECK_TAG(AsyncConnection_TAG, connection_ref)
    connection_ref->read_state = READ_STATE_ACTIVE;
    LOG_FMT("reader read_state: %d %s", connection_ref->read_state, read_state_str(connection_ref->read_state));
    if(connection_ref->read_state != READ_STATE_ACTIVE) {
        printf("reader read_state is : %d %s\n", connection_ref->read_state, async_read_state_str(connection_ref->read_state));
    }

    if(connection_ref->read_state == READ_STATE_STOP) {
        connection_ref->handler_ref->handle_reader_stopped(connection_ref->handler_ref); //, NULL, AsyncConnectionErrCode_is_closed);
    }
    int buffer_size = connection_ref->read_buffer_size;
    if (connection_ref->active_input_buffer_ref == NULL) {
        connection_ref->active_input_buffer_ref = IOBuffer_new_with_capacity(buffer_size);
    } else {
        IOBuffer_reset(connection_ref->active_input_buffer_ref);
    }
    IOBufferRef iob = connection_ref->active_input_buffer_ref;
    void *input_buffer_ptr = IOBuffer_space(iob);
    int input_buffer_length = IOBuffer_space_len(iob);
    int fd = connection_ref->socket_stream_ref->fd;
    int bytes_available = IOBuffer_data_len(iob);
    bytes_available = recv(fd, input_buffer_ptr, input_buffer_length, MSG_DONTWAIT);
    if(bytes_available > 0)
        IOBuffer_commit(iob, bytes_available);
    int errno_save = errno;
    LOG_FMT("bytes_available %d errno: %d %s", bytes_available, errno_save, strerror(errno_save));
    int eagain = EAGAIN;
    char* errstr = strerror(errno_save);
    if(bytes_available > 0) {
        LOG_FMT("Before AsyncParser_consume read_state %s", read_state_str(connection_ref->read_state));
        int ec = http_parser_consume(connection_ref->http_parser_ptr, IOBuffer_data(iob), IOBuffer_data_len(iob));
        llhttp_errno_t llhttp_ec = ec;
        LOG_FMT("After AsyncParser_consume returns  errno: %d errno_str: %s  read_state %s ", errno_save, strerror(errno_save), read_state_str(connection_ref->read_state));
        if(errno_save == EAGAIN) {
            bool x = llhttp_message_needs_eof(connection_ref->http_parser_ptr->m_llhttp_ptr);
            LOG_FMT("test this result message_needs_eof %d", (int)x);
            read_eagain(connection_ref);
            return;
        }
        if(ec == HPE_OK) {
            // out of data start another read
//            CHTTP_ASSERT((connection_ref->read_state == READ_STATE_ACTIVE), "something is wrong");
            read_need_data(connection_ref);
        } else {
            read_error(connection_ref, "parser error");
        }
    } else if(bytes_available == 0) {
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
static void read_eagain(AsyncConnectionRef cref)
{
    CHECK_TAG(AsyncConnection_TAG, cref)
    LOG_FMT("read_eagain connection_ref->read_state: %s", read_state_str(cref->read_state));
    cref->read_state = READ_STATE_EAGAINED;
}
static void read_need_data(AsyncConnectionRef cref)
{
    CHECK_TAG(AsyncConnection_TAG, cref)
    cref->read_state = READ_STATE_IDLE;
    LOG_FMT("read_need_data connection_ref->read_state: %s", read_state_str(cref->read_state));
    async_post_to_reactor(cref, &postable_reader);
}
static void postable_read_cb(ReactorRef reactor_ref, void* arg)
{
    AsyncConnectionRef connection_ref = arg;

}
static void read_error(AsyncConnectionRef connection_ref, char* msg)
{
    CHECK_TAG(AsyncConnection_TAG, connection_ref)
    LOG_FMT("Read_error got an error this is the message: %s  fd: %d", msg, connection_ref->socket_stream_ref->fd);
    connection_ref->read_state = READ_STATE_STOP;
    connection_ref->handler_ref->handle_reader_stopped(connection_ref->handler_ref);
    async_post_to_reactor(connection_ref, &async_postable_cleanup);
}

llhttp_errno_t async_on_read_message_complete(http_parser_t* parser_ptr, MessageRef msg)
{
    CHECK_TAG(HTTP_PARSER_TAG, parser_ptr)
    AsyncConnectionRef connection_ref = parser_ptr->handler_context;
    CHECK_TAG(AsyncConnection_TAG, connection_ref)
    AsyncHandlerRef handler_ref = connection_ref->handler_ref;
    CHECK_TAG(AsyncHandler_TAG, handler_ref)
    CHTTP_ASSERT((msg != NULL), "msg should not be NULL");
    connection_ref->handler_ref->handle_request(handler_ref, msg);
    LOG_FMT("read_message_handler - on_write_cb  read_state: %s", read_state_str(connection_ref->read_state));
    return HPE_OK;
}
static MessageRef reply_invalid_request(AsyncConnectionRef connection_ref, const char* error_message)
{
    CHECK_TAG(AsyncConnection_TAG, connection_ref)
    MessageRef m = Message_new();
    Message_set_is_request(m, false);
    BufferChainRef bdy =  BufferChain_new();
    BufferChain_append_cstr(bdy, "You made a mistake. message: ");
    char tmp[100];
    sprintf(tmp, "%s", error_message);
    BufferChain_append_cstr(bdy, tmp);
    Message_set_body(m, bdy);
    return m;
}
