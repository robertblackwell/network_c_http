
#define _GNU_SOURCE
#include <c_http/demo_protocol/demo_connection.h>
#include <c_http/simple_runloop/rl_internal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <errno.h>
#include <c_http/macros.h>
#include <c_http//logger.h>
#include <c_http/simple_runloop/runloop.h>
#include <c_http/demo_protocol/demo_message.h>

#define READ_STATE_IDLE     11
#define READ_STATE_EAGAINED 12
#define READ_STATE_ACTIVE   13

#define WRITE_STATE_IDLE     21
#define WRITE_STATE_EAGAINED 22
#define WRITE_STATE_ACTIVE   23

static void event_handler(RtorStreamRef stream_ref, uint64_t event);
static void write_epollout(DemoConnectionRef connection_ref);
static void read_epollin(DemoConnectionRef connection_ref);

static void read_start(DemoConnectionRef connection_ref);
static void postable_reader(ReactorRef reactor_ref, void* arg);
static void reader(DemoConnectionRef connection_ref);

static void postable_writer(ReactorRef reactor_ref, void* arg);
static void postable_write_call_cb(ReactorRef reactor_ref, void* arg);
static void writer(DemoConnectionRef connection_ref);

static void read_error(DemoConnectionRef connection_ref, char* message);
static void write_error(DemoConnectionRef connection_ref, char* message);
static void on_read_complete(DemoConnectionRef connection_ref, DemoMessageRef msg, int error_code);
static void read_parser_error_handler(DemoConnectionRef connection_ref, const char* error_message);
static DemoMessageRef reply_invalid_request(DemoConnectionRef connection_ref, const char* error_message);

void iobuffer_dealloc(void** p)
{
    IOBuffer_destroy(*p);
    *p == NULL;
}

DemoConnectionRef democonnection_new(
        int socket,
        ReactorRef reactor_ref,
        DemoHandlerRef handler_ref,
        void(*connection_completion_cb)(void* href))
{
    DemoConnectionRef this = malloc(sizeof(DemoConnection));
    democonnection_init(this, socket, reactor_ref, handler_ref, connection_completion_cb);
    return this;
}
void democonnection_init(
        DemoConnectionRef this,
        int socket,
        ReactorRef reactor_ref,
        DemoHandlerRef handler_ref,
        void(*connection_completion_cb)(void* href))
{
    DEMO_CONNECTION_SET_TAG(this)
    DEMO_CONNECTION_CHECK_TAG(this)
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
    this->on_write_cb = NULL;
    this->on_read_cb = NULL;
    this->on_close_cb = connection_completion_cb;
    this->parser_ref = DemoParser_new(
            (void*)&on_read_complete,
            this);
    rtor_stream_register(this->socket_stream_ref);
    this->socket_stream_ref->both_arg = this;
    rtor_stream_arm_both(this->socket_stream_ref, &event_handler, this);
}
void democonnection_free(DemoConnectionRef this)
{
    DEMO_CONNECTION_CHECK_TAG(this)
    rtor_stream_free(this->socket_stream_ref);
    free(this);
}
/////////////////////////////////////////////////////////////////////////////////////
// event handler
///////////////////////////////////////////////////////////////////////////////////////
static void event_handler(RtorStreamRef stream_ref, uint64_t event)
{
    LOG_FMT("event_handler %lx\n", event);
    DemoConnectionRef connection_ref = stream_ref->context;
    DEMO_CONNECTION_CHECK_TAG(connection_ref)
    LOG_FMT("demohandler handler \n");
    if(event & EPOLLOUT) {
        write_epollout(connection_ref);
    }
    if(event & EPOLLIN) {
        read_epollin(connection_ref);
    } else {

    }
}
static void read_epollin(DemoConnectionRef connection_ref)
{
    DEMO_CONNECTION_CHECK_TAG(connection_ref)
    if(connection_ref->read_state == READ_STATE_EAGAINED) {
        connection_ref->read_state = READ_STATE_ACTIVE;
        read_start(connection_ref);
    }
}
static void write_epollout(DemoConnectionRef connection_ref)
{
    DEMO_CONNECTION_CHECK_TAG(connection_ref)
    if(connection_ref->write_state == WRITE_STATE_EAGAINED) {
        connection_ref->write_state = WRITE_STATE_ACTIVE;
        connection_ref->writeside_posted = true;
        rtor_reactor_post(connection_ref->reactor_ref, &postable_writer, connection_ref);
    }
}

/////////////////////////////////////////////////////////////////////////////////////
// read sequence
////////////////////////////////////////////////////////////////////////////////////
void democonnection_read(DemoConnectionRef connection_ref, void(*on_demo_read_cb)(void* href, DemoMessageRef, int statuc))
{
    LOG_FMT("democonnect_read\n");
    CHTTP_ASSERT((connection_ref->read_state != READ_STATE_ACTIVE), "a read is already active");
    if(connection_ref->read_state == READ_STATE_IDLE) {
        connection_ref->read_state = READ_STATE_ACTIVE;
        connection_ref->on_read_cb = on_demo_read_cb;
        read_start(connection_ref);
    } else if(connection_ref->read_state == READ_STATE_EAGAINED) {
        connection_ref->on_read_cb = on_demo_read_cb;
    } 
}
static void read_start(DemoConnectionRef connection_ref)
{
    DEMO_CONNECTION_CHECK_TAG(connection_ref)
    connection_ref->readside_posted = true;
    rtor_reactor_post(connection_ref->reactor_ref, &postable_reader, connection_ref);
}
static void postable_reader(ReactorRef reactor_ref, void* arg)
{
    DemoConnectionRef connection_ref = arg;
    LOG_FMT("postable_reader read_state: %d\n", connection_ref->read_state);
    connection_ref->readside_posted = false;
    reader(connection_ref);
}
static void reader(DemoConnectionRef connection_ref) {
    DEMO_CONNECTION_CHECK_TAG(connection_ref)
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
        LOG_FMT("Before DemoParser_consume read_state %d\n", connection_ref->read_state);
        DemoParserPrivateReturnValue rv = DemoParser_consume(connection_ref->parser_ref, iob);
        LOG_FMT("After DemoParser_consume returns error_code: %d  errno: %d read_state %d \n", rv.error_code, errno_save, connection_ref->read_state);
        if(rv.error_code == 0) {
            if(connection_ref->read_state == READ_STATE_ACTIVE) {
                LOG_FMT("reader post postable_reader connection_ref->read_state: %d\n", connection_ref->read_state);
                rtor_reactor_post(connection_ref->reactor_ref, &postable_reader, connection_ref);
            }
        } else {
        }

    } else if(bytes_available == 0) {
        /**
         * TODO - handle close of the socket and release of the related data structures
         */
        read_error(connection_ref, "reader zero bytes - peer closed connection");

    } else {
        if (errno_save == eagain) {
            connection_ref->read_state = READ_STATE_EAGAINED;
        } else {
            /**
             * TODO - handle close of the socket and release of the related data structures
             */
            read_error(connection_ref, "reader - io error close and move on");
        }
    }
    LOG_FMT("reader return\n");
}
static void postable_read_cb(ReactorRef reactor_ref, void* arg)
{
    DemoConnectionRef connection_ref = arg;

}
static void on_read_complete(DemoConnectionRef connection_ref, DemoMessageRef msg, int error_code)
{
    DEMO_CONNECTION_CHECK_TAG(connection_ref)
    CHTTP_ASSERT( (((msg != NULL) && (error_code == 0)) || ((msg == NULL) && (error_code != 0))), "msg != NULL and error_code == 0 failed OR msg == NULL and error_code != 0 failed");
    connection_ref->read_state = READ_STATE_IDLE;
    LOG_FMT("read_message_handler - on_write_cb  read_state: %d\n", connection_ref->read_state);
    DC_Read_CB tmp = connection_ref->on_read_cb;
    /**
     * Need the on_read_cb property NULL befor going further
     */
    connection_ref->on_read_cb = NULL;
    tmp(connection_ref->handler_ref, msg, error_code);
}
/////////////////////////////////////////////////////////////////////////////////////
// end of read sequence
/////////////////////////////////////////////////////////////////////////////////////
// write sequence
////////////////////////////////////////////////////////////////////////////////////
void democonnection_write(
        DemoConnectionRef connection_ref,
        DemoMessageRef response_ref,
        void(*on_demo_write_cb)(void* href, int statuc))
{
    DEMO_CONNECTION_CHECK_TAG(connection_ref)
    CHTTP_ASSERT((response_ref != NULL), "got NULL instead of a response_ref");
    CHTTP_ASSERT((on_demo_write_cb != NULL), "got NULL for on_demo_write_cb");
    CHTTP_ASSERT((connection_ref->write_state == WRITE_STATE_IDLE), "a write is already active");
    connection_ref->on_write_cb = on_demo_write_cb;
    connection_ref->write_state == WRITE_STATE_ACTIVE;
    connection_ref->active_output_buffer_ref = demo_message_serialize(response_ref);
    rtor_reactor_post(connection_ref->reactor_ref, &postable_writer, connection_ref);
}
static void postable_writer(ReactorRef reactor_ref, void* arg)
{
    DemoConnectionRef connection_ref = arg;
    DEMO_CONNECTION_CHECK_TAG(connection_ref)
    CHTTP_ASSERT((connection_ref->active_output_buffer_ref != NULL), "post_write_handler");
    CHTTP_ASSERT((connection_ref->write_state == WRITE_STATE_IDLE), "cannot call write while write state is not idle");
    writer(connection_ref);
}

static void writer(DemoConnectionRef connection_ref)
{
    DEMO_CONNECTION_CHECK_TAG(connection_ref)
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
            rtor_reactor_post(connection_ref->reactor_ref, &postable_write_call_cb, connection_ref);
        } else {
            // write not complete schedule another try
            connection_ref->write_state = WRITE_STATE_ACTIVE;
            rtor_reactor_post(connection_ref->reactor_ref, &postable_writer, connection_ref);
        }
    } else if (wrc == 0) {
        write_error(connection_ref, "think the fd is closed by other end");
    } else if ((wrc == -1) && (errno == EAGAIN)) {
        connection_ref->write_state = WRITE_STATE_EAGAINED;
        return;
    } else if(wrc == -1) {
        error(connection_ref, "think this was an io error");
    }
}
static void post_to_reactor(DemoConnectionRef connection_ref, void(*postable_function)(ReactorRef, void*))
{
    rtor_reactor_post(connection_ref->reactor_ref, postable_function, connection_ref);
}
static void postable_write_call_cb(ReactorRef reactor_ref, void* arg)
{
    DemoConnectionRef connection_ref = arg;
    DEMO_CONNECTION_CHECK_TAG(connection_ref)
    CHTTP_ASSERT((connection_ref->on_write_cb != NULL), "write call back is NULL");
    connection_ref->on_write_cb(connection_ref->handler_ref, 0);
}
static void write_error(DemoConnectionRef connection_ref, char* msg)
{
    DEMO_CONNECTION_CHECK_TAG(connection_ref)
    printf("Got an error this is the message: %s  fd: %d\n", msg, connection_ref->socket_stream_ref->fd);
    rtor_stream_deregister(connection_ref->socket_stream_ref);
    close(connection_ref->socket_stream_ref->fd);
    connection_ref->on_close_cb(connection_ref->handler_ref);
}

static void read_error(DemoConnectionRef connection_ref, char* msg)
{
    DEMO_CONNECTION_CHECK_TAG(connection_ref)
    printf("Got an error this is the message: %s  fd: %d\n", msg, connection_ref->socket_stream_ref->fd);
    rtor_stream_deregister(connection_ref->socket_stream_ref);
    close(connection_ref->socket_stream_ref->fd);
    connection_ref->on_close_cb(connection_ref->handler_ref);
}
static DemoMessageRef reply_invalid_request(DemoConnectionRef connection_ref, const char* error_message)
{
    DEMO_CONNECTION_CHECK_TAG(connection_ref)
    DemoMessageRef m = demo_message_new();
    demo_message_set_is_request(m, false);
    BufferChainRef bdy =  BufferChain_new();
    BufferChain_append_cstr(bdy, "You made a mistake. message: ");
    char tmp[100];
    sprintf(tmp, "%s", error_message);
    BufferChain_append_cstr(bdy, tmp);
    demo_message_set_body(m, bdy);
    return m;
}
static DemoMessageRef process_request(DemoConnectionRef connection_ref, DemoMessageRef request)
{
    DEMO_CONNECTION_CHECK_TAG(connection_ref)
    DemoMessageRef reply = demo_message_new();
//    DemoMessageRef request = connection_ref->parser_ref->m_current_message_ptr;
    demo_message_set_is_request(reply, false);
    BufferChainRef request_body = demo_message_get_body(request);
    BufferChainRef bc = BufferChain_new();
    BufferChain_append_bufferchain(bc, request_body);
    demo_message_set_body(reply, bc);
}