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

/**
 * Means that there is probably data available to read (that is not definitely EAGAIN)
 * on the socket but a read has not yet be initiated.
 *
 * Input queue is empty.
 *
 * It is legal to issue/accept a call to async_connection_read() in this state,
 * and such a call will initiate a new read sequence.
 */
#define READ_STATE_IDLE     11

/**
 * Like IDLE except that the input queue is NOT empty.
 * It is legal to issue/accept async_connection_read() and it will return (via callback)
 * a message from the input queue.
 *
 * Should never get to this state
 */
#define READ_STATE_INPUT_PENDING 51

/**
 * A read got  errno == EAGAIN and have not yet recieved a subsequent POLLIN from epoll.
 * The errno == EAGAIN may be:
 * -    nread > 0 and errno == EAGAIN - which means a partial read but there is no more
 * -    nread < 0 and errno == EAGAIN - means there was nothing to read
 * -    nread == 0 is reserved for when peer closes the socket
 *
 * This state implies a read is in progress. As only get to this state if a read
 * gets EAGAIN and a message did not complete.
 *
 * Hence nread > 0 && errno == EAGAIN and message completed which is the same as
 * input queue not empty should cause the pending read to be satisfied
 * (via posting the read callback so that it runs later) and the state to be ACTIVE
 *
 * Conditon - after sending the message to the readers callback the inputqueue should be empty.
 *
 * We should only have one message in the queue at a time
 *
 * Hence it is illegal to issue/accept a call to async_connection_read() in this state.
 */
#define READ_STATE_EAGAINED 12

/**
 * A read sequence has been started but has not yet completed a full message.
 * The function reader() will set the read state to ACTIVE to signify a read sequence is started.
 * The on_message_complete() function will set the state to IDLE (or add the message to the input queue
 * so that the input queue is NOT empty - need to decide on this signalling mechanism)
 * to signify another message has been received and has been added to the input list.
 *
 * The readers callback should be posted with the input message and the input queue
 * should then be empty
 *
 * A precondition for moving to this state is that the inpt queue is empty.
 *
 * Should not start a read while unprocessed input messages are on the input-queue
 */
#define READ_STATE_ACTIVE   13

/**
 * This state indicates that a read sequence is active and that the function on_message_complete()
 * has detected a new message and added it to the input queue.
 *
 * This state is equivalent so READ_STATE_ACTIVE && (is_empty_input_queue != true)
 */
#define READ_STATE_ACTIVE_NEWMESSAGE 16

/**
 * Signifies the socket is no longer available.
 * Either:
 * -    nread == 0 - the peer closed the connection or
 * -    nread < 0 and errno != EAGAIN signifies an IO error
 * -    return code from http_parser_consume() is not HPE_OK
 */
#define READ_STATE_STOP     14


#define READ_STATE_POSTED_READER 15

#define WRITE_STATE_IDLE     21
#define WRITE_STATE_EAGAINED 22
#define WRITE_STATE_ACTIVE   23
#define WRITE_STATE_STOP     24


void async_event_handler(RtorStreamRef stream_ref, uint64_t event);

void async_post_to_reactor(AsyncConnectionRef connection_ref, void(*postable_function)(ReactorRef, void*));

void async_read_start(AsyncConnectionRef connection_ref);
llhttp_errno_t async_on_read_message_complete(http_parser_t* parser_ptr, MessageRef msg);

void async_postable_writer(ReactorRef reactor_ref, void* arg);
void async_postable_write_call_cb(ReactorRef reactor_ref, void* arg);

void async_postable_cleanup(ReactorRef reactor, void* cref);
const char* async_read_state_str(int state);
const char* async_write_state_str(int state);
const char* async_epoll_event_str(int event);

#endif