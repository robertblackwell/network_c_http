#ifndef chttp_demo_connection_h
#define chttp_demo_connection_h
#include <src/runloop/runloop.h>
#include <src/common/list.h>
#include <src/common/iobuffer.h>
#include <src/demo_protocol/demo_message_parser.h>
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
    RunloopRef           runloop_ref;
    AsioStreamRef        asio_stream_ref;
    void*                handler_ref;
    ListRef              input_message_list_ref;
    IOBufferRef          active_input_buffer_ref;
    long                 read_buffer_size;
    IOBufferRef          active_output_buffer_ref;
    DemoMessageParserRef parser_ref;
    int                  read_state;
    int                  write_state;
    DC_Read_CB           on_read_cb;
    void*                on_read_cb_arg;
    DC_Write_CB          on_write_cb;
    void*                on_write_cb_arg;
    DC_Close_CB          on_close_cb;
    void*                on_close_cb_arg;
    bool                 cleanup_done_flag;
    RBL_DECLARE_END_TAG;

} DemoConnection, *DemoConnectionRef;

DemoConnectionRef demo_connection_new(
        RunloopRef runloop_ref,
        int socket,
        void(*connection_completion_cb)(void* href),
        void* handler_ref
        );
void demo_connection_init(
        DemoConnectionRef this,
        RunloopRef runloop_ref,
        int socket,
        void(*connection_completion_cb)(void* href),
        void* handler_ref
        );
void demo_connection_free(DemoConnectionRef this);
void demo_connection_read(DemoConnectionRef connection_ref, void(*on_demo_read_cb)(void* href, DemoMessageRef, int status), void* href);
void demo_connection_write(DemoConnectionRef connection_ref, DemoMessageRef, void(*on_demo_write_cb)(void* href, int status), void* href);
void demo_connection_close(DemoConnectionRef cref);
#endif