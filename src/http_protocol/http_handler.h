#ifndef C_HTTP_Http_HANDLER_H
#define C_HTTP_Http_HANDLER_H
#include <rbl/check_tag.h>
#include <runloop/runloop.h>
#include <common/list.h>
#include <common/iobuffer.h>
#include <http_protocol/http_connection.h>
#include "http_server.h"

#define HttpHandler_TAG "DmHDLR"
#include <runloop/runloop.h>

typedef struct HttpHandler_s HttpHandler, *HttpHandlerRef;

/**
 * Instance of a HttpHamdler calls this CB when it is closing down.
 * void* server_ref is an anonymous reference passed at handler creation by its parent object
 */
typedef void(*DH_Completion_CB)(void* server_ref, HttpHandlerRef href);

typedef struct HttpHandler_s {
    RBL_DECLARE_TAG;
    int               raw_socket;
    RunloopRef        runloop_ref;
    HttpConnectionRef http_connection_ref;
    HttpProcessRequestFunction request_handler;
    DH_Completion_CB  completion_callback;
    void*             server_ref;
    ListRef           input_list;
    ListRef           output_list; // List of HttpMessageRef - responses
    HttpMessageRef    active_response;
    RBL_DECLARE_END_TAG;

} HttpHandler, *HttpHandlerRef;

HttpHandlerRef http_handler_new(
        RunloopRef runloop_ref,
        int socket,
        HttpProcessRequestFunction request_handler,
        void(*completion_cb)(void*, HttpHandlerRef),
        void* completion_cb_arg);
void http_handler_init(
        HttpHandlerRef this,
        RunloopRef runloop_ref,
        int socket,
        HttpProcessRequestFunction request_handler,
        void(*completion_cb)(void*, HttpHandlerRef),
        void* completion_cb_arg);
void http_handler_free(HttpHandlerRef this);

#endif