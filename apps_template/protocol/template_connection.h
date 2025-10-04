#ifndef chttp_tmpl_connection_h
#define chttp_tmpl_connection_h
#include <src/runloop/runloop.h>
#include <src/common/list.h>
#include <src/common/iobuffer.h>
#include <src/tmpl_protocol/tmpl_message_parser.h>
#include <rbl/check_tag.h>

#define TmplConnection_TAG "DmCONN"
#include <rbl/check_tag.h>

enum TmplConnectionErrCode {
    TmplConnectionErrCode_is_closed = -31,
    TmplConnectionErrCode_io_error = -32,
    TmplConnectionErrCode_parse_error = -33
};

typedef struct TmplConnection_s TmplConnection, *TmplConnectionRef;
/**
 * In the following void* href is aan anonymous reference
 * passed to the connection at init time by the parent/sibling object
 * that created the TmplConnection object
 *
 */
typedef void(*DC_Read_CB)(void* href, TmplMessageRef, int status);
typedef void(*DC_Write_CB)(void* href, int status);
/**
 * DC_Close_CB is called by the connection when it is closing down
 */
typedef void(*DC_Close_CB)(void* href);

typedef struct TmplConnection_s {
    RBL_DECLARE_TAG;
    RunloopRef           runloop_ref;
    AsioStreamRef        asio_stream_ref;
    /**
     * handler_ref should be of type DemoHandlerRef but has been made
     * anonymous to get around a circular dependency in header files
     * between demo_connection.h and demo_handler.h
     */
    void*                handler_ref;
    ListRef              input_message_list_ref;
    IOBufferRef          active_input_buffer_ref;
    long                 read_buffer_size;
    IOBufferRef          active_output_buffer_ref;
    TmplMessageParserRef parser_ref;
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
} TmplConnection, *TmplConnectionRef;

TmplConnectionRef tmpl_connection_new(
        RunloopRef runloop_ref,
        int socket,
        void(*connection_completion_cb)(void* href),
        void* handler_ref
        );
void tmpl_connection_init(
        TmplConnectionRef this,
        RunloopRef runloop_ref,
        int socket,
        void(*connection_completion_cb)(void* href),
        void* handler_ref
        );
void tmpl_connection_free(TmplConnectionRef this);
void tmpl_connection_read(TmplConnectionRef connection_ref, void(*on_tmpl_read_cb)(void* href, TmplMessageRef, int status), void* href);
void tmpl_connection_write(TmplConnectionRef connection_ref, TmplMessageRef, void(*on_tmpl_write_cb)(void* href, int status), void* href);
void tmpl_connection_close(TmplConnectionRef cref);
#endif