#ifndef C_HTTP_DEMO_HANDLER_H
#define C_HTTP_DEMO_HANDLER_H
#include <rbl/check_tag.h>
#include <http_in_c/runloop/rl_internal.h>
#include <http_in_c/common/list.h>
#include <http_in_c/common/iobuffer.h>
#include <http_in_c/demo_protocol/demo_server.h>
#include <http_in_c/demo_protocol/demo_connection.h>

#define DemoHandler_TAG "DmHDLR"
#include <http_in_c/runloop/rl_checktag.h>

typedef struct DemoHandler_s DemoHandler, *DemoHandlerRef;

/**
 * Instance of a DemoHamdler calls this CB when it is closing down.
 * void* server_ref is an anonymous reference passed at handler creation by its parent object
 */
typedef void(*DH_Completion_CB)(void* server_ref, DemoHandlerRef href);

typedef struct DemoHandler_s {
    RBL_DECLARE_TAG;
    int                 raw_socket;
    RunloopRef          reactor_ref;
    DemoConnectionRef   demo_connection_ref;
    DH_Completion_CB    completion_callback;
//    void(*completion_callback)(void* server_ref, DemoHandlerRef href);
    void*               server_ref;
    ListRef             input_list;
    ListRef             output_list; // List of DemoMessageRef - responses
    DemoMessageRef      active_response;
    RBL_DECLARE_END_TAG;

} DemoHandler, *DemoHandlerRef;

DemoHandlerRef demohandler_new(
        int socket,
        RunloopRef reactor_ref,
        void(*completion_cb)(void*, DemoHandlerRef),
        void* completion_cb_arg);
void demohandler_init(
        DemoHandlerRef this, int socket,
        RunloopRef reactor_ref,
        void(*completion_cb)(void*, DemoHandlerRef),
        void* completion_cb_arg);
void demohandler_free(DemoHandlerRef this);
void demohandler_anonymous_dispose(void** p);

#endif