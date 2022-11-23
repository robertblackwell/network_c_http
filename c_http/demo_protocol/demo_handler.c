
#define _GNU_SOURCE
#include <c_http/demo_protocol/demo_handler.h>
#include <c_http/simple_runloop/rl_internal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <errno.h>
#include <c_http/macros.h>
#include <c_http/simple_runloop/runloop.h>
#include <c_http/demo_protocol/demo_message.h>



typedef struct Input_s {

}Input, *InputRef;

static void read_start(DemoHandlerRef handler_ref);
static void post_read_handler(ReactorRef reactor_ref, void* arg);
static void read_epollin(DemoHandlerRef handler_ref);
static void postable_read_handler(ReactorRef reactor_ref, void* arg);

// a function that can be posted to the reactor runlie
static void postable_write_handler(ReactorRef reactor_ref, void* arg);

static void write_start_maybe(DemoHandlerRef handler_ref);
static void write_epollout(DemoHandlerRef handler_ref);

static void post_handler(ReactorRef reactor_ref, void* arg);
static void event_handler(RtorStreamRef stream_ref, uint64_t event);
static void reader(DemoHandlerRef handler_ref);
static void writer(DemoHandlerRef handler_ref);
static void handler(RtorStreamRef stream_ref, uint64_t event);
static void error(DemoHandlerRef href, char* message);
static DemoMessageRef reply_invalid_request(DemoHandlerRef href, DemoParserReturnValue rv);
static DemoMessageRef process_request(DemoHandlerRef href, DemoMessageRef request);
//static void handler_post(DemoHandlerRef handler_ref)
//{
//    handler_ref->post_active = true;
//    rtor_reactor_post(handler_ref->reactor_ref, post_handler, handler_ref);
//}


void iobuffer_dealloc(void** p)
{
    IOBuffer_destroy(*p);
    *p == NULL;
}

DemoHandlerRef demohandler_new(int socket, ReactorRef reactor_ref, DemoServerRef server_ref)
{
    DemoHandlerRef this = malloc(sizeof(DemoHandler));
    demohandler_init(this, socket, reactor_ref, server_ref);
}
void demohandler_init(DemoHandlerRef this, int socket, ReactorRef reactor_ref, DemoServerRef server_ref)
{
    DEMO_HANDLER_SET_TAG(this)
    DEMO_HANDLER_CHECK_TAG(this)
    this->raw_socket = socket;
    this->reactor_ref = reactor_ref;
    this->server_ref = server_ref;
    this->socket_stream_ref = rtor_stream_new(reactor_ref, socket);
    this->socket_stream_ref->context = this;
    this->active_input_buffer_ref = NULL;
    this->active_output_buffer_ref = NULL;
    this->input_list = List_new(NULL);
    this->output_list = List_new(NULL);
    this->parser_ref = DemoParser_new();
    rtor_stream_register(this->socket_stream_ref);
    this->socket_stream_ref->both_arg = this;
    rtor_stream_arm_both(this->socket_stream_ref, &event_handler, this);
    read_start(this);
}
void democlient_close(DemoHandlerRef this)
{

}
void demohandler_free(DemoHandlerRef this)
{
    DEMO_HANDLER_CHECK_TAG(this)
    rtor_stream_free(this->socket_stream_ref);
    free(this);
}
static void post_handler(ReactorRef reactor_ref, void* arg)
{
    DemoHandlerRef handler_ref = arg;
    DEMO_HANDLER_CHECK_TAG(handler_ref)
    handler_ref->post_active = false;
    if(!handler_ref->write_eagained) {
        writer(handler_ref);
    } else if(!handler_ref->read_eagained) {
        reader(handler_ref);
    }
    if(handler_ref->read_eagained && handler_ref->write_eagained) {
        // ensure we wait for an event
    }

}
static void event_handler(RtorStreamRef stream_ref, uint64_t event)
{
    printf("event_handler %lx\n", event);
    DemoHandlerRef handler_ref = stream_ref->context;
    DEMO_HANDLER_CHECK_TAG(handler_ref)
    printf("demohandler handler \n");
    if(event & EPOLLOUT) {
        write_epollout(handler_ref);
    }
    if(event & EPOLLIN) {
        if (handler_ref->read_eagained) {
            read_epollin(handler_ref);
        }
    } else {

    }
}
/**
 * Checks to see whether:
 * -    there is a response pending that needs to be serialized
 * -    and a write sequence started
 * Not to be called in the event handler as a result of a epollout
 */
static void write_start_maybe(DemoHandlerRef handler_ref)
{
    DEMO_HANDLER_CHECK_TAG(handler_ref)
    if(handler_ref->active_output_buffer_ref == NULL) {
        DemoResponseRef resp = List_remove_first(handler_ref->output_list);
        if(resp != NULL) {
            handler_ref->active_output_buffer_ref = demo_message_serialize(resp);
            if(!handler_ref->write_eagained) {
                rtor_reactor_post(handler_ref->reactor_ref, &postable_write_handler, handler_ref);
            }
        }
    }
}
/**
 * Should be called on event epollout.
 * This function will set the handler_ref->write_eagained flag
 * Will post a write_post_handler if required
 * If nothing to write or write_eagained ends the sequence of posts
 */
static void write_epollout(DemoHandlerRef handler_ref)
{
    DEMO_HANDLER_CHECK_TAG(handler_ref)
    if(handler_ref->active_output_buffer_ref != NULL) {
        if(handler_ref->write_eagained) {
            // need to restart write
            handler_ref->write_eagained = false;
            rtor_reactor_post(handler_ref->reactor_ref, &postable_write_handler, handler_ref);
        }
    } else {
        handler_ref->write_eagained = false;
        DemoResponseRef resp = List_first(handler_ref->output_list);
        if(resp != NULL) {
            handler_ref->active_output_buffer_ref = demo_message_serialize(resp);
            if(handler_ref->write_eagained) {
                rtor_reactor_post(handler_ref->reactor_ref, &postable_write_handler, handler_ref);
            }
        }
    }
}
static void writer(DemoHandlerRef handler_ref)
{
    DEMO_HANDLER_CHECK_TAG(handler_ref)
    CHTTP_ASSERT((handler_ref->active_output_buffer_ref != NULL), "writer");
    IOBufferRef iob = handler_ref->active_output_buffer_ref;

    long wrc = send(handler_ref->socket_stream_ref->fd, IOBuffer_data(iob), IOBuffer_data_len(iob), MSG_DONTWAIT);
    if(wrc > 0) {
        handler_ref->write_eagained = false;
        IOBuffer_consume(iob, wrc);
        if(IOBuffer_data_len(iob) == 0) {
            IOBuffer_destroy(iob);
            handler_ref->active_output_buffer_ref = NULL;
            write_start_maybe(handler_ref);
        } else {
            rtor_reactor_post(handler_ref->reactor_ref, &postable_write_handler, handler_ref);
        }
    } else if (wrc == 0) {
        error(handler_ref, "think the fd is closed by other end");
    } else if ((wrc == -1) && (errno == EAGAIN)) {
        handler_ref->write_eagained = true;
        return;
    } else if(wrc == -1) {
        error(handler_ref, "think this was an io error");
    }
}
static void postable_write_handler(ReactorRef reactor_ref, void* arg)
{
    DemoHandlerRef handler_ref = arg;
    DEMO_HANDLER_CHECK_TAG(handler_ref)
    CHTTP_ASSERT((handler_ref->active_output_buffer_ref != NULL), "post_write_handler");
    writer(handler_ref);
}
static void postable_read_handler(ReactorRef reactor_ref, void* arg)
{
    DemoHandlerRef handler_ref = arg;
    reader(handler_ref);
}
static void read_start(DemoHandlerRef handler_ref)
{
    DEMO_HANDLER_CHECK_TAG(handler_ref)
    rtor_reactor_post(handler_ref->reactor_ref, &postable_read_handler, handler_ref);
}
static void postable_reader(ReactorRef reactor_ref, void* arg)
{
    DemoHandlerRef handler_ref = arg;
    DEMO_HANDLER_CHECK_TAG(handler_ref)
    reader(handler_ref);
}
static void reader(DemoHandlerRef handler_ref) {
    DEMO_HANDLER_CHECK_TAG(handler_ref)
    int buffer_size = 1000;
    IOBufferRef iob;
    void *input_buffer_ptr;
    int input_buffer_length;
    int data_length_still_in_iobuffer;
    if (handler_ref->active_input_buffer_ref == NULL) {
        handler_ref->active_input_buffer_ref = IOBuffer_new_with_capacity(buffer_size);
        if(handler_ref->parser_ref->m_current_message_ptr == NULL) {
            DemoMessageRef m = demo_message_new();
            DemoParser_begin(handler_ref->parser_ref, m);
        }
    }
#if 0
    if (IOBuffer_space_len(handler_ref->active_input_buffer_ref) == 0) {
#if 1
        IOBuffer_consolidate_space(handler_ref->active_input_buffer_ref);
#else
        IOBufferRef tmp = handler_ref->active_input_buffer_ref;
        handler_ref->active_input_buffer_ref = IOBuffer_new_with_capacity(buffer_size);
        IOBuffer_data_add(handler_ref->active_input_buffer_ref, IOBuffer_data(tmp), IOBuffer_data_len(tmp));
        IOBuffer_dispose(&tmp);
#endif
    }
#endif
    iob = handler_ref->active_input_buffer_ref;
    input_buffer_ptr = IOBuffer_space(iob);
    input_buffer_length = IOBuffer_space_len(iob);
    data_length_still_in_iobuffer = IOBuffer_data_len(handler_ref->active_input_buffer_ref);
    int fd = handler_ref->socket_stream_ref->fd;
    int bytes_available;
    if (data_length_still_in_iobuffer == 0) {
        // no data left in buffer after previous read and parse - read more
        bytes_available = recv(fd, input_buffer_ptr, input_buffer_length, MSG_DONTWAIT);
        if(bytes_available > 0)
            IOBuffer_commit(iob, bytes_available);
    } else {
        // there is data still in the buffer from the last parse operation
        // that means the last parse stopped short probably end-of-message
        // just parse the remaining data
        bytes_available = data_length_still_in_iobuffer;
        input_buffer_ptr = IOBuffer_data(handler_ref->active_input_buffer_ref);
    }
    int errno_save = errno;
    int eagain = EAGAIN;
    char* errstr = strerror(errno_save);
    if(bytes_available > 0) {
        DemoParserReturnValue rv = DemoParser_consume(handler_ref->parser_ref, input_buffer_ptr, bytes_available);
        IOBuffer_consume(iob, rv.bytes_consumed);
        DemoMessageRef response = NULL;
        if(rv.error_code == 0) {
            if (rv.eom_flag) {
                DemoMessageRef request = handler_ref->parser_ref->m_current_message_ptr;
                List_add_back(handler_ref->input_list, request);
                response = process_request(handler_ref, request);
//                demo_message_dispose(&(handler_ref->parser_ref->m_current_message_ptr));
//                handler_ref->parser_ref->m_current_message_ptr = NULL;
                handler_ref->parser_ref->m_current_message_ptr = demo_message_new();
                List_add_back(handler_ref->output_list, response);
                if(IOBuffer_data_len(handler_ref->active_input_buffer_ref) == 0) {
                    // if the buffer has not been fully processed leave the remainder for the next iteration
                    IOBuffer_dispose(&(handler_ref->active_input_buffer_ref));
                }
                write_start_maybe(handler_ref);
//                demo_message_dispose(&(handler_ref->parser_ref->m_current_message_ptr));
//                handler_ref->parser_ref->m_current_message_ptr = NULL;
//                handler_ref->parser_ref->m_current_message_ptr = demo_message_new();
            } else {
                // incomplete message
            }
        }else {
            // TODO - really reply to invalid messages ? probably not
            response = reply_invalid_request(handler_ref, rv);
            List_add_back(handler_ref->output_list, response);
            if(IOBuffer_data_len(handler_ref->active_input_buffer_ref) == 0) {
                // if the buffer has not been fully processed leave the remainder for the next iteration
                IOBuffer_dispose(&(handler_ref->active_input_buffer_ref));
            }
            write_start_maybe(handler_ref);
            demo_message_dispose(&(handler_ref->parser_ref->m_current_message_ptr));
            handler_ref->parser_ref->m_current_message_ptr = NULL;
            handler_ref->parser_ref->m_current_message_ptr = demo_message_new();
        }
//        if(response != NULL) {
//            List_add_back(handler_ref->output_list, response);
//            if(IOBuffer_data_len(handler_ref->active_input_buffer_ref) == 0) {
//                // if the buffer has not been fully processed leave the remainder for the next iteration
//                IOBuffer_dispose(&(handler_ref->active_input_buffer_ref));
//            }
//            write_start_maybe(handler_ref);
//            demo_message_dispose(&(handler_ref->parser_ref->m_current_message_ptr));
//            handler_ref->parser_ref->m_current_message_ptr = NULL;
//            handler_ref->parser_ref->m_current_message_ptr = demo_message_new();
//        }
        rtor_reactor_post(handler_ref->reactor_ref, &postable_reader, handler_ref);

    } else if(bytes_available == 0) {
        /**
         * TODO - handle close of the socket and release of the related data structures
         */
        error(handler_ref, "reader zero bytes - peer closed connection");
    } else {
        if (errno_save == eagain) {
            handler_ref->read_eagained = true;
        } else {
            /**
             * TODO - handle close of the socket and release of the related data structures
             */
            error(handler_ref, "reader - io error close and move on");
        }
    }
}
static void read_epollin(DemoHandlerRef handler_ref)
{
    DEMO_HANDLER_CHECK_TAG(handler_ref)
    handler_ref->read_eagained = false;
    if(handler_ref->active_input_buffer_ref == NULL) {
        read_start(handler_ref);
    } else {
        read_start(handler_ref);
    }
}
static void queue_response(DemoHandlerRef handler_ref, DemoResponseRef response_ref)
{
    DEMO_HANDLER_CHECK_TAG(handler_ref)
    List_add_back(handler_ref->output_list, response_ref);
    write_start_maybe(handler_ref);
}

static void error(DemoHandlerRef href, char* msg)
{
    DEMO_HANDLER_CHECK_TAG(href)
    printf("Got an error this is the message: %s\n", msg);
}
static DemoMessageRef reply_invalid_request(DemoHandlerRef href, DemoParserReturnValue rv)
{
    DEMO_HANDLER_CHECK_TAG(href)
    DemoMessageRef m = demo_message_new();
    demo_message_set_is_request(m, false);
    BufferChainRef bdy =  BufferChain_new();
    BufferChain_append_cstr(bdy, "You made a mistake");
    demo_message_set_body(m, bdy);
    return m;
}
static DemoMessageRef process_request(DemoHandlerRef href, DemoMessageRef request)
{
    DEMO_HANDLER_CHECK_TAG(href)
    DemoMessageRef reply = demo_message_new();
//    DemoMessageRef request = href->parser_ref->m_current_message_ptr;
    demo_message_set_is_request(reply, false);
    BufferChainRef request_body = demo_message_get_body(request);
    BufferChainRef bc = BufferChain_new();
    BufferChain_append_bufferchain(bc, request_body);
    demo_message_set_body(reply, bc);
}