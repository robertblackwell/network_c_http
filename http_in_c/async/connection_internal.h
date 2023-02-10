#ifndef async_connection_h
#define async_connection_h

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


void event_handler(RtorStreamRef stream_ref, uint64_t event);
llhttp_errno_t on_read_message_complete(http_parser_t* parser_ptr, MessageRef msg);
void post_to_reactor(AsyncConnectionRef connection_ref, void(*postable_function)(ReactorRef, void*));

//static void write_epollout(AsyncConnectionRef connection_ref);
//static void read_epollin(AsyncConnectionRef connection_ref);
//
void read_start(AsyncConnectionRef connection_ref);
//static void postable_reader(ReactorRef reactor_ref, void* arg);
//static void reader(AsyncConnectionRef connection_ref);
//static llhttp_errno_t on_read_message_complete(http_parser_t* parser_ptr, MessageRef msg);
//static void read_error(AsyncConnectionRef connection_ref, char* msg);
//static void read_eagain(AsyncConnectionRef cref);
//static void read_need_data(AsyncConnectionRef cref);
//
void postable_writer(ReactorRef reactor_ref, void* arg);
void postable_write_call_cb(ReactorRef reactor_ref, void* arg);
//static void writer(AsyncConnectionRef connection_ref);
//static void write_error(AsyncConnectionRef connection_ref, char* msg);
//
void postable_cleanup(ReactorRef reactor, void* cref);
//const char* read_state_str(int state);
//const char* write_state_str(int state);

/**
 * Utility function that wraps all rtor_reactor_post() calls so this module can
 * keep track of outstanding pending function calls
 */
void post_to_reactor(AsyncConnectionRef connection_ref, void(*postable_function)(ReactorRef, void*));

#endif