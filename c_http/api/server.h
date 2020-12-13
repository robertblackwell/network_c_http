#ifndef c_http_server_h
#define c_http_server_h

#include <c_http/constants.h>
#include <c_http/dsl/queue.h>
#include <c_http/details/worker.h>
#include <c_http/api/message.h>
#include <c_http/api/writer.h>
#include <c_http/socket_functions.h>
#include <c_http/api/handler.h>
#include <c_http/api/handler_example.h>

#define TYPE Server
#define Server_TAG "SERVER"
#include <c_http/check_tag.h>
#undef TYPE
#define SERVER_DECLARE_TAG DECLARE_TAG(Server)
#define SERVER_CHECK_TAG(p) CHECK_TAG(Server, p)
#define SERVER_SET_TAG(p) SET_TAG(Server, p)


/**
 * Server - a synchronous server as an opaque object
 */
typedef struct  Server_s Server, *ServerRef;

/**
 * Create a new server object.
 * \param port     The localhost:port on which the server will listen
 * \param handler  A function conforming to HandlerFunction (see handler.h) which will be called to handle all requests that are parsed successfully.
 * \return
 */
ServerRef Server_new(int port, HandlerFunction handler);
void Server_free(ServerRef* srefptr);
void Server_listen(ServerRef server);
void Server_terminate(ServerRef this);
#endif