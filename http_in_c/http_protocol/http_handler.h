#ifndef C_HTTP_DEMO_HANDLER_H
#define C_HTTP_DEMO_HANDLER_H
#include <rbl/check_tag.h>
#include <http_in_c/runloop/rl_internal.h>
#include <http_in_c/common/list.h>
#include <http_in_c/common/iobuffer.h>
#include <http_in_c/http_protocol/http_connection.h>
#include <http_in_c/http_protocol/http_server.h>

#define HttpHandler_TAG "DmHDLR"
#include <http_in_c/runloop/rl_checktag.h>

typedef struct HttpHandler_s HttpHandler, *HttpHandlerRef;

/**
 * Instance of a HttpHamdler calls this CB when it is closing down.
 * void* server_ref is an anonymous reference passed at handler creation by its parent object
 */
typedef void(*DH_Completion_CB)(void* server_ref, HttpHandlerRef href);

typedef struct HttpHandler_s {
    RBL_DECLARE_TAG;
    int                 raw_socket;
    RunloopRef          runloop_ref;
    HttpConnectionRef   http_connection_ref;
    DH_Completion_CB    completion_callback;
    void*               server_ref;
    ListRef             input_list;
    ListRef             output_list; // List of HttpMessageRef - responses
    HttpMessageRef      active_response;
    RBL_DECLARE_END_TAG;

} HttpHandler, *HttpHandlerRef;

HttpHandlerRef demohandler_new(
        RunloopRef runloop_ref,
        int socket,
        void(*completion_cb)(void*, HttpHandlerRef),
        void* completion_cb_arg);
void demohandler_init(
        HttpHandlerRef this,
        RunloopRef runloop_ref,
        int socket,
        void(*completion_cb)(void*, HttpHandlerRef),
        void* completion_cb_arg);
void demohandler_free(HttpHandlerRef this);

#endif