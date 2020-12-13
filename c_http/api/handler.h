#ifndef c_http_handler_h
#define c_http_handler_h

#include <c_http/constants.h>
#include <c_http/dsl/queue.h>
#include <c_http/api/message.h>
#include <c_http/api/writer.h>
#include <c_http/details/worker.h>
#include <c_http/socket_functions.h>
#include <c_http/api/handler_example.h>

/**
 * This is the signature of a handler function. *The address) of such a function must
 * be provided to Server_new() in order that the server and its worker threads
 * can call on this function to handle requests.
 *
 * The handler function is called once for each request and is passed the request message in its
 * entirety together with a Writer instance that provides functions to write the response.
 *
 * The handler function is solely responsible for constructing and sending the response.
 */
typedef int(*HandlerFunction)(MessageRef request, WriterRef wrttr);

#endif