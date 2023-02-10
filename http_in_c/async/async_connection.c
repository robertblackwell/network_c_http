
#define _GNU_SOURCE
#define CHLOG_ON
#include <http_in_c/async/async.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <errno.h>
#include <http_in_c/macros.h>
#include <http_in_c/logger.h>
#include <http_in_c/http/message.h>

#define READ_STATE_IDLE     11
#define READ_STATE_EAGAINED 12
#define READ_STATE_ACTIVE   13
#define READ_STATE_STOP     14

#define WRITE_STATE_IDLE     21
#define WRITE_STATE_EAGAINED 22
#define WRITE_STATE_ACTIVE   23
#define WRITE_STATE_STOP     24

static void event_handler(RtorStreamRef stream_ref, uint64_t event);
static void write_epollout(AsyncConnectionRef connection_ref);
static void read_epollin(AsyncConnectionRef connection_ref);

static void read_start(AsyncConnectionRef connection_ref);
static void postable_reader(ReactorRef reactor_ref, void* arg);
static void reader(AsyncConnectionRef connection_ref);
static llhttp_errno_t on_read_message_complete(http_parser_t* parser_ptr, MessageRef msg);
static void read_error(AsyncConnectionRef connection_ref, char* msg);
static void read_eagain(AsyncConnectionRef cref);
static void read_need_data(AsyncConnectionRef cref);

static void postable_writer(ReactorRef reactor_ref, void* arg);
static void postable_write_call_cb(ReactorRef reactor_ref, void* arg);
static void writer(AsyncConnectionRef connection_ref);
static void write_error(AsyncConnectionRef connection_ref, char* msg);

static void postable_cleanup(ReactorRef reactor, void* cref);
const char* read_state_str(int state);
const char* write_state_str(int state);

/**
 * Utility function that wraps all rtor_reactor_post() calls so this module can
 * keep track of outstanding pending function calls
 */
static void post_to_reactor(AsyncConnectionRef connection_ref, void(*postable_function)(ReactorRef, void*));


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
    ParserOnMessageCompleteHandler h = on_read_message_complete;
    this->http_parser_ptr = http_parser_new(h, this);
    rtor_stream_register(this->socket_stream_ref);
    this->socket_stream_ref->both_arg = this;
    rtor_stream_arm_both(this->socket_stream_ref, &event_handler, this);
    this->read_buffer_size = 1000000;
}
void async_connection_free(AsyncConnectionRef this)
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
    free(this);
}
/////////////////////////////////////////////////////////////////////////////////////
// event handler called from the Reactor on receiving an epoll event
///////////////////////////////////////////////////////////////////////////////////////
static void event_handler(RtorStreamRef stream_ref, uint64_t event)
{
    LOG_FMT("event_handler %lx", event);
    AsyncConnectionRef connection_ref = stream_ref->context;
    CHECK_TAG(AsyncConnection_TAG, connection_ref)
    LOG_FMT("async_handler handler");
    if(event & EPOLLOUT) {
        write_epollout(connection_ref);
    }
    if(event & EPOLLIN) {
        read_epollin(connection_ref);
    }
    if(!((event & EPOLLIN) || (event & EPOLLOUT))){
        LOG_FMT("not EPOLLIN and not EPOLLOUT")
    }
}
static void read_epollin(AsyncConnectionRef connection_ref)
{
    CHECK_TAG(AsyncConnection_TAG, connection_ref)
    LOG_FMT("read_epollin read_state: %s", read_state_str(connection_ref->read_state))
    if(connection_ref->read_state == READ_STATE_EAGAINED) {
        connection_ref->read_state = READ_STATE_ACTIVE;
        read_start(connection_ref);
    }
}
static void write_epollout(AsyncConnectionRef connection_ref)
{
    CHECK_TAG(AsyncConnection_TAG, connection_ref)
    LOG_FMT("write_epollout")
    if(connection_ref->write_state == WRITE_STATE_EAGAINED) {
        connection_ref->write_state = WRITE_STATE_ACTIVE;
        connection_ref->writeside_posted = true;
        post_to_reactor(connection_ref,&postable_writer);
    }
}

/////////////////////////////////////////////////////////////////////////////////////
// read sequence - sequence of functions called processing a read operation
////////////////////////////////////////////////////////////////////////////////////
void async_connection_read(AsyncConnectionRef connection_ref) //, void(*on_read_message_cb)(void* href, MessageRef, int status))
{
    CHECK_TAG(AsyncConnection_TAG, connection_ref)
    LOG_FMT("async_connection_read read_state: %d %s", connection_ref->read_state, read_state_str(connection_ref->read_state));
//    CHTTP_ASSERT((connection_ref->read_state != READ_STATE_ACTIVE), "a read is already active");
    if(connection_ref->read_state == READ_STATE_IDLE) {
        connection_ref->read_state = READ_STATE_ACTIVE;
        read_start(connection_ref);
    } else if(connection_ref->read_state == READ_STATE_EAGAINED) {
//        connection_ref->on_read_message_cb = on_read_message_cb;
    } else if(connection_ref->read_state == READ_STATE_STOP) {
        connection_ref->handler_ref->handle_reader_stopped(connection_ref->handler_ref); //, NULL, AsyncConnectionErrCode_is_closed);
    }
}
static void read_start(AsyncConnectionRef connection_ref)
{
    CHECK_TAG(AsyncConnection_TAG, connection_ref)
    connection_ref->readside_posted = true;
    post_to_reactor(connection_ref, &postable_reader);
}
static void postable_reader(ReactorRef reactor_ref, void* arg)
{
    AsyncConnectionRef connection_ref = arg;
    LOG_FMT("postable_reader read_state: %d %s", connection_ref->read_state, read_state_str(connection_ref->read_state));
    if(connection_ref->read_state == READ_STATE_STOP) {
        connection_ref->handler_ref->handle_reader_stopped(connection_ref->handler_ref);//, NULL, AsyncConnectionErrCode_is_closed);
        return;
    }
    reader(connection_ref);
}
static void reader(AsyncConnectionRef connection_ref) {
    CHECK_TAG(AsyncConnection_TAG, connection_ref)
    LOG_FMT("reader read_state: %d %s", connection_ref->read_state, read_state_str(connection_ref->read_state));
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
            printf("test this result message_needs_eof %d\n", (int)x);
            read_eagain(connection_ref);
            return;
        }
        if(ec == HPE_OK) {
            // out of data start another read
//            CHTTP_ASSERT((connection_ref->read_state == READ_STATE_ACTIVE), "something is wrong");
            read_need_data(connection_ref);
        } else {
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
    post_to_reactor(cref, &postable_reader);
}
#if 0
static void reader_old(AsyncConnectionRef connection_ref) {
    CHECK_TAG(AsyncConnection_TAG, connection_ref)
    if(connection_ref->read_state == READ_STATE_STOP) {
        connection_ref->on_read_cb(connection_ref->handler_ref, NULL, AsyncConnectionErrCode_is_closed);
    }
    int buffer_size = 1000;
    if (connection_ref->active_input_buffer_ref == NULL) {
        connection_ref->active_input_buffer_ref = IOBuffer_new_with_capacity(buffer_size);
    }
    IOBufferRef iob = connection_ref->active_input_buffer_ref;
    void *input_buffer_ptr = IOBuffer_space(iob);
    int input_buffer_length = IOBuffer_space_len(iob);
    int fd = connection_ref->socket_stream_ref->fd;
    int bytes_available = IOBuffer_data_len(iob);
    if (bytes_available == 0) {
        bytes_available = recv(fd, input_buffer_ptr, input_buffer_length, MSG_DONTWAIT);
        if(bytes_available > 0)
            IOBuffer_commit(iob, bytes_available);
    }
    int errno_save = errno;
    int eagain = EAGAIN;
    char* errstr = strerror(errno_save);
    if(bytes_available > 0) {
        LOG_FMT("Before AsyncParser_consume read_state %d\n", connection_ref->read_state);
        // TODO experiment with generic programming in C
//        int ec = connection_ref->http_parser_ptr->parser_consume((ParserInterfaceRef)connection_ref->parser_ref, iob);
        int ec = http_parser_consume(connection_ref->http_parser_ptr, IOBuffer_data(iob), IOBuffer_data_len(iob));
        LOG_FMT("After AsyncParser_consume returns  errno: %d read_state %d \n", errno_save, connection_ref->read_state);
        if(ec == 0) {
            if(connection_ref->read_state == READ_STATE_ACTIVE) {
                LOG_FMT("reader post postable_reader connection_ref->read_state: %d\n", connection_ref->read_state);
                post_to_reactor(connection_ref, &postable_reader);
            }
        } else {
        }
    } else if(bytes_available == 0) {
        read_error(connection_ref, "reader zero bytes - peer closed connection");
    } else if (errno_save == eagain) {
        connection_ref->read_state = READ_STATE_EAGAINED;
    } else {
        read_error(connection_ref, "reader - io error close and move on");
    }
    LOG_FMT("reader return\n");
}
#endif
static void postable_read_cb(ReactorRef reactor_ref, void* arg)
{
    AsyncConnectionRef connection_ref = arg;

}
static llhttp_errno_t on_read_message_complete(http_parser_t* parser_ptr, MessageRef msg)
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
/////////////////////////////////////////////////////////////////////////////////////
// end of read sequence
/////////////////////////////////////////////////////////////////////////////////////
// write sequence - sequence of functions called during write operation
////////////////////////////////////////////////////////////////////////////////////
void async_connection_write(AsyncConnectionRef connection_ref, MessageRef response_ref)
//        void(*on_write_message_cb)(void* href, int statuc))
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
    post_to_reactor(connection_ref, &postable_writer);
}
static void postable_writer(ReactorRef reactor_ref, void* arg)
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
    if(wrc > 0) {
        IOBuffer_consume(iob, wrc);
        if(IOBuffer_data_len(iob) == 0) {
            // write is complete
            connection_ref->write_state = WRITE_STATE_IDLE;
            IOBuffer_dispose(&(connection_ref->active_output_buffer_ref));
            connection_ref->active_output_buffer_ref = NULL;
            post_to_reactor(connection_ref, &postable_write_call_cb);
        } else {
            // write not complete schedule another try
            connection_ref->write_state = WRITE_STATE_ACTIVE;
            post_to_reactor(connection_ref, &postable_writer);
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
static void post_to_reactor(AsyncConnectionRef connection_ref, void(*postable_function)(ReactorRef, void*))
{
    rtor_reactor_post(connection_ref->reactor_ref, postable_function, connection_ref);
}
static void postable_write_call_cb(ReactorRef reactor_ref, void* arg)
{
    AsyncConnectionRef connection_ref = arg;
    CHECK_TAG(AsyncConnection_TAG, connection_ref)
    AsyncHandlerRef href = connection_ref->handler_ref;
    href->handle_write_done(href);
}
/////////////////////////////////////////////////////////////////////////////////////
// end of read sequence
/////////////////////////////////////////////////////////////////////////////////////
// Error functions
/////////////////////////////////////////////////////////////////////////////////////

static void write_error(AsyncConnectionRef connection_ref, char* msg)
{
    CHECK_TAG(AsyncConnection_TAG, connection_ref)
    printf("Write_error got an error this is the message: %s  fd: %d\n", msg, connection_ref->socket_stream_ref->fd);
    connection_ref->write_state = WRITE_STATE_STOP;
    connection_ref->handler_ref->handle_write_failed(connection_ref->handler_ref);
    post_to_reactor(connection_ref, &postable_cleanup);
}
static void read_error(AsyncConnectionRef connection_ref, char* msg)
{
    CHECK_TAG(AsyncConnection_TAG, connection_ref)
    printf("Read_error got an error this is the message: %s  fd: %d\n", msg, connection_ref->socket_stream_ref->fd);
    connection_ref->read_state = READ_STATE_STOP;
    connection_ref->handler_ref->handle_reader_stopped(connection_ref->handler_ref);
    post_to_reactor(connection_ref, &postable_cleanup);
}
/////////////////////////////////////////////////////////////////////////////////////
// end of error functions
/////////////////////////////////////////////////////////////////////////////////////
// cleanup sequence - functions called when connection is terminating
/////////////////////////////////////////////////////////////////////////////////////

/**
 * This must be the last async connection function to run and it should only run once.
 */
static void postable_cleanup(ReactorRef reactor, void* cref)
{
    AsyncConnectionRef connection_ref = cref;
    printf("postable_cleanup entered\n");
    CHTTP_ASSERT((connection_ref->cleanup_done_flag == false), "cleanup should not run more than once");
    CHECK_TAG(AsyncConnection_TAG, connection_ref)
    rtor_stream_deregister(connection_ref->socket_stream_ref);
    connection_ref->handler_ref->handle_connection_done(connection_ref->handler_ref);
}
/////////////////////////////////////////////////////////////////////////////////////
// end of cleanup
/////////////////////////////////////////////////////////////////////////////////////
// process request - DEPRECATED ... I think. Look in async_handler.c for
// these functions.
/////////////////////////////////////////////////////////////////////////////////////
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
#if 0
static MessageRef process_request(AsyncHandlerRef handler_ref, MessageRef request)
{
    CHECK_TAG(AsyncHandler_TAG, handler_ref);
    CHECK_TAG(AsyncConnection_TAG, handler_ref->async_connection_ref)
    MessageRef reply = Message_new();
//    AsyncMessageRef request = connection_ref->parser_ref->m_current_message_ptr;
    Message_set_is_request(reply, false);
    BufferChainRef request_body = Message_get_body(request);
    BufferChainRef bc = BufferChain_new();
    BufferChain_append_bufferchain(bc, request_body);
    Message_set_body(reply, bc);
}
#endif

const char* read_state_str(int state)
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
        default:
            CHTTP_ASSERT(false, "Invalid read state");
    }
}
const char* write_state_str(int state)
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
            CHTTP_ASSERT(false, "Invalid read state");
    }
}
const char* epoll_event_str(int event)
{
    return "";
}