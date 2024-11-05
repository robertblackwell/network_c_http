#ifndef chttp_demo_connection_h
#define chttp_demo_connection_h
#include <http_in_c/runloop/runloop.h>
#include <http_in_c/common/list.h>
#include <http_in_c/common/iobuffer.h>
#include <http_in_c/demo_protocol/demo_server.h>
//#include <http_in_c/demo_protocol/demo_handler.h>
#include <http_in_c/demo_protocol/demo_parser.h>
#include <rbl/check_tag.h>

#define DemoConnection_TAG "DmCONN"
#include <rbl/check_tag.h>

enum DemoConnectionErrCode {
    DemoConnectionErrCode_is_closed = -31,
    DemoConnectionErrCode_io_error = -32,
    DemoConnectionErrCode_parse_error = -33
};

typedef struct DemoConnection_s DemoConnection, *DemoConnectionRef;
/**
 * In the following void* href is aan anonymous reference
 * passed to the connection at init time by the parent/sibling object
 * that created the DemoConnection object
 *
 */
typedef void(*DC_Read_CB)(void* href, DemoMessageRef, int status);
typedef void(*DC_Write_CB)(void* href, int status);
/**
 * DC_Close_CB is called by the connection when it is closing down
 */
typedef void(*DC_Close_CB)(void* href);

typedef struct DemoConnection_s {
    RBL_DECLARE_TAG;
    RunloopRef       reactor_ref;
    RunloopStreamRef socket_stream_ref;
    DemoHandlerRef   handler_ref;
    IOBufferRef      active_input_buffer_ref;
    IOBufferRef      active_output_buffer_ref;
    DemoParserRef    parser_ref;
    int              read_state;
    int              write_state;
    DC_Read_CB       on_read_cb;
    DC_Write_CB      on_write_cb;
    DC_Close_CB      on_close_cb;
    bool             cleanup_done_flag;
    bool             readside_posted;
    bool             writeside_posted;
    bool             post_active;
    RBL_DECLARE_END_TAG;

} DemoConnection, *DemoConnectionRef;

DemoConnectionRef democonnection_new(
        int socket,
        RunloopRef runloop_ref,
        DemoHandlerRef handler_ref,
        void(*connection_completion_cb)(void* href));
void democonnection_init(
        DemoConnectionRef this,
        int socket,
        RunloopRef runloop_ref,
        DemoHandlerRef handler_ref,
        void(*connection_completion_cb)(void* href));
void democonnection_free(DemoConnectionRef this);
void democonnection_amonymous_dispose(void* p);

void democonnection_read(DemoConnectionRef connection_ref, void(*on_demo_read_cb)(void* href, DemoMessageRef, int statuc));
void democonnection_write(DemoConnectionRef connection_ref, DemoMessageRef, void(*on_demo_write_cb)(void* href, int statuc));

#endif