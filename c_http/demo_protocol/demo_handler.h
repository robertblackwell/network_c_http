#ifndef chttp_demo_handler_h
#define chttp_demo_handler_h
#include <c_http/simple_runloop/runloop.h>
#include <c_http/common/list.h>
#include <c_http/common/iobuffer.h>
#include <c_http/demo_protocol/demo_server.h>
#include <c_http/demo_protocol/demo_connection.h>

#define TYPE DemoHandler
#define DemoHandler_TAG "DmHDLR"
#include <c_http/check_tag.h>
#define DEMO_HANDLER_DECLARE_TAG DECLARE_TAG(DemoHandler_TAG)
#define DEMO_HANDLER_CHECK_TAG(p) CHECK_TAG(DemoHandler_TAG, p)
#define DEMO_HANDLER_SET_TAG(p) SET_TAG(DemoHandler_TAG, p)
#undef TYPE



typedef struct DemoHandler_s DemoHandler, *DemoHandlerRef;

/**
 * Instance of a DemoHamdler calls this CB when it is closing down.
 * void* server_ref is an anonymous reference passed at handler creation by its parent object
 */
typedef void(*DH_Completion_CB)(void* server_ref, DemoHandlerRef href);

typedef struct DemoHandler_s {
    DEMO_PARSER_DECLARE_TAG;
    int                 raw_socket;
    ReactorRef          reactor_ref;
    DemoConnectionRef   demo_connection_ref;
    DH_Completion_CB    completion_callback;
//    void(*completion_callback)(void* server_ref, DemoHandlerRef href);
    void*               server_ref;
    ListRef             input_list;
    ListRef             output_list; // List of DemoMessageRef - responses

} DemoHandler, *DemoHandlerRef;

DemoHandlerRef demohandler_new(
        int socket,
        ReactorRef reactor_ref,
        void(*completion_cb)(void*, DemoHandlerRef),
        void* completion_cb_arg);
void demohandler_init(
        DemoHandlerRef this, int socket,
        ReactorRef reactor_ref,
        void(*completion_cb)(void*, DemoHandlerRef),
        void* completion_cb_arg);
void demohandler_free(DemoHandlerRef this);
void demohandler_amonymous_dispose(void* p);

#endif