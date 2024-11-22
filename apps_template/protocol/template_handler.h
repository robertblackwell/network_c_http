#ifndef C_HTTP_Tmpl_HANDLER_H
#define C_HTTP_Tmpl_HANDLER_H
#include <rbl/check_tag.h>
#include <http_in_c/runloop/rl_internal.h>
#include <http_in_c/common/list.h>
#include <http_in_c/common/iobuffer.h>
#include <http_in_c/tmpl_protocol/tmpl_connection.h>
#include <http_in_c/tmpl_protocol/tmpl_server.h>

#define TmplHandler_TAG "DmHDLR"
#include <http_in_c/runloop/rl_checktag.h>

typedef struct TmplHandler_s TmplHandler, *TmplHandlerRef;

/**
 * Instance of a TmplHamdler calls this CB when it is closing down.
 * void* server_ref is an anonymous reference passed at handler creation by its parent object
 */
typedef void(*DH_Completion_CB)(void* server_ref, TmplHandlerRef href);

typedef struct TmplHandler_s {
    RBL_DECLARE_TAG;
    int               raw_socket;
    RunloopRef        runloop_ref;
    TmplConnectionRef tmpl_connection_ref;
    TmplProcessRequestFunction request_handler;
    DH_Completion_CB  completion_callback;
    void*             server_ref;
    ListRef           input_list;
    ListRef           output_list; // List of TmplMessageRef - responses
    TmplMessageRef    active_response;
    RBL_DECLARE_END_TAG;

} TmplHandler, *TmplHandlerRef;

TmplHandlerRef tmpl_handler_new(
        RunloopRef runloop_ref,
        int socket,
        DemoProcessRequestFunction request_handler,
        void(*completion_cb)(void*, TmplHandlerRef),
        void* completion_cb_arg);
void tmpl_handler_init(
        TmplHandlerRef this,
        RunloopRef runloop_ref,
        int socket,
        DemoProcessRequestFunction request_handler,
        void(*completion_cb)(void*, TmplHandlerRef),
        void* completion_cb_arg);
void tmpl_handler_free(TmplHandlerRef this);

#endif