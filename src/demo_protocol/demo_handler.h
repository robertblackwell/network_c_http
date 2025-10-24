#ifndef C_HTTP_Demo_HANDLER_H
#define C_HTTP_Demo_HANDLER_H
#include <rbl/check_tag.h>
// #include <src/runloop/rl_internal.h>
#include <src/common/list.h>
#include <src/common/iobuffer.h>
#include <src/demo_protocol/demo_connection.h>
#include <src/demo_protocol/demo_server.h>

#define DemoHandler_TAG "DmHDLR"
// #include <src/runloop/rl_checktag.h>

typedef struct DemoHandler_s DemoHandler, *DemoHandlerRef;

/**
 * Instance of a DemoHamdler calls this CB when it is closing down.
 * void* server_ref is an anonymous reference passed at handler creation by its parent object
 */
typedef void(*DH_Completion_CB)(void* server_ref, DemoHandlerRef href);

typedef struct DemoHandler_s {
    RBL_DECLARE_TAG;
    int               raw_socket;
    RunloopRef        runloop_ref;
    DemoConnectionRef demo_connection_ref;
    DemoProcessRequestFunction request_handler;
    DH_Completion_CB  completion_callback;
    void*             server_ref;
    ListRef           input_list;
    ListRef           output_list; // List of DemoMessageRef - responses
    DemoMessageRef    active_response;
    RBL_DECLARE_END_TAG;

} DemoHandler, *DemoHandlerRef;

DemoHandlerRef demo_handler_new(
        RunloopRef runloop_ref,
        int socket,
        DemoProcessRequestFunction request_handler,
        void(*completion_cb)(void*, DemoHandlerRef),
        void* completion_cb_arg);
void demo_handler_init(
        DemoHandlerRef this,
        RunloopRef runloop_ref,
        int socket,
        DemoProcessRequestFunction request_handler,
        void(*completion_cb)(void*, DemoHandlerRef),
        void* completion_cb_arg);
void demo_handler_free(DemoHandlerRef this);

#endif