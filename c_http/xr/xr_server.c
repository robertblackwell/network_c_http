#define _GNU_SOURCE

#include <c_http/xr/xr_server.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>

#include <c_http/constants.h>
#include <c_http/alloc.h>
#include <c_http/utils.h>

#include <c_http/logger.h>
#include <c_http/socket_functions.h>
#include <c_http/xr/evfd_queue.h>
#include<c_http/xr/conn.h>
#include <c_http/xr/socket_watcher.h>
#include <sys/epoll.h>


//
// create a listening socket from host and port
//
static socket_handle_t create_listener_socket(int port, const char *host)
{

    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    socket_handle_t tmp_socket;
    sin.sin_family = AF_INET; // or AF_INET6 (address family)
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = inet_addr("127.0.0.1");
    int result;
    int yes = 1;

    if((tmp_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        goto error_01;
    }

    // sin.sin_len = sizeof(sin);
    if((result = setsockopt(tmp_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))) != 0) {
        goto error_02;
    }

    if((result = bind(tmp_socket, (struct sockaddr *) &sin, sizeof(sin))) != 0) {
        goto error_03;
    }

    if((result = listen(tmp_socket, SOMAXCONN)) != 0) {
        goto error_04;
    }
    return tmp_socket;

    error_01:
    printf("socket call failed with errno %d \n", errno);
    assert(0);
    error_02:
    printf("setsockopt call failed with errno %d \n", errno);
    assert(0);
    error_03:
    printf("bind call failed with errno %d \n", errno);
    assert(0);
    error_04:
    printf("listen call failed with errno %d \n", errno);
    assert(0);
}

void set_non_blocking(socket_handle_t socket)
{
    //
    // Ensure socket is in blocking mode
    //
    int flags = fcntl(socket, F_GETFL, 0);
    int modFlags2 = flags | O_NONBLOCK;
    int fres = fcntl(socket, F_SETFL, modFlags2);
    assert(fres == 0);
}
typedef enum Reader_ReturnCode {
    READER_OK = 0,             // A message was returned
    READER_PARSE_ERROR = -1,   // An error in the format of the message was detected.
    READER_IO_ERROR = -2,      // An IO error occurred.
    READER_EAGAIN = -3,
} Reader_ReturnCode;

// first parameter is really an XrSocketWatchRef - but this is one of the problems with inheritence in C
static void state_machine(XrWatcherRef wp, void *arg, uint64_t event)
{

    XrSocketWatcherRef sw = (XrSocketWatcherRef)wp;
    XrConnRef conn_ref = arg;
    XrReactorRef reactor_ref = sw->runloop;

#define NEXT_STATE(state) \
    conn_ref->state = state; \
    Xrsw_change_watch(sw, &state_machine, arg, 0); \
    XrReactor_post(reactor_ref, sw, &state_machine, arg);

    printf("XrWorker::wrkr_state_machine fd: %d\n", conn_ref->fd);
    uint64_t e1 = EPOLLIN;
    uint64_t e2 = EPOLLOUT;
    uint64_t e3 = EPOLLERR;
    uint64_t e4 = EPOLLHUP;
    uint64_t e5 = EPOLLRDHUP;
    bool pollin = (event & EPOLLIN);
    bool pollout = (event & EPOLLOUT);
    bool pollerr = (event & EPOLLERR);
    bool pollhup = (event & EPOLLHUP);
    bool pollrdhup = (event & EPOLLRDHUP);

    switch(conn_ref->state) {
        case XRCONN_STATE_UNINIT:assert(false);
            break;

        case XRCONN_STATE_RDINIT: {
            XR_PRINTF("connection state machine RDINIT\n");
            XrConn_prepare_read(conn_ref);
            conn_ref->state = XRCONN_STATE_READ;
            XrReactor_post(reactor_ref, wp, &state_machine, arg);
        }
        break;
        case XRCONN_STATE_READ: {
            assert(conn_ref->req_msg_ref != NULL);
            assert(conn_ref->parser_ref != NULL);
            XrReadRC rc = XrConn_read(conn_ref);
            // have to decide what next
            XR_PRINTF("XrServer::state_machine after read rc :%d \n", rc);
            if (rc == XRD_EAGAIN) {
                XR_PRINTF("XrServer::state_machine EAGAIN\n");
                return;
            } else {
                if(rc == XRD_EOM) {
                    XR_PRINTF("XrServer::state_machine EOM\n");
                    assert(conn_ref->parser_ref->m_message_done);
                    assert(conn_ref->req_msg_ref != NULL);
                    assert(conn_ref->req_msg_ref == conn_ref->parser_ref->m_current_message_ptr);
                    conn_ref->state = XRCONN_STATE_HANDLE;
                } else if(rc == XRD_EOF) {
                    XR_PRINTF("XrServer::state_machine EOF\n");
                    conn_ref->state = XRCONN_STATE_CLOSE;
                } else if(rc == XRD_ERROR) {
                    XR_PRINTF("XrServer::state_machine XRD_ERROR\n");
                    conn_ref->state = XRCONN_STATE_ERROR;
                } else {
                    XR_PRINTF("XrServer::state_machine XRD_PERROR\n");
                    assert(rc == XRD_PERROR);
                    conn_ref->state = XRCONN_STATE_BADREQ;
                }
                Xrsw_change_watch(sw, &state_machine, arg, 0);
                XrReactor_post(reactor_ref, wp, &state_machine, arg);
                return;
            }
        }
        break;

        case XRCONN_STATE_HANDLE: {
            CbufferRef ser = Message_serialize(conn_ref->req_msg_ref);
            BufferChainRef body = Message_get_body(conn_ref->req_msg_ref);
            CbufferRef cbody = BufferChain_compact(body);
            int blen = BufferChain_size(body);
            XR_PRINTF("XrServer::state_machine STATE_HANDLE new msg body length : %d\n", blen);

            XrHandlerRef handler = XrHandler_new(conn_ref);
        }
        break;

        case XRCONN_STATE_WRITE_STATUS_INIT: {
            IOBufferRef status_line_buf = XrHandler_status_line(conn_ref->handler_ref);
            XrConn_write_2(conn_ref, status_line_buf, &state_machine);
            if(conn_ref->write_rc == XRW_COMPLETE) {
                conn_ref->state = XRCONN_STATE_WRITE_HDRLINE_INIT;
            } else {
                conn_ref->state = XRCONN_STATE_ERROR;
            }
            Xrsw_change_watch(sw, &state_machine, arg, 0);
            XrReactor_post(reactor_ref, wp, &state_machine, arg);
        }
        break;

        case XRCONN_STATE_WRITE_STATUS:
            // process output data
            break;

        case XRCONN_STATE_WRITE_HDRLINE_INIT:
            // process output data
            break;

        case XRCONN_STATE_WRITE_HDRLINE:
            // process output data
            break;

        case XRCONN_STATE_WRITE_BODY_INIT:
            // process output data
            break;

        case XRCONN_STATE_WRITE_BODY:
            // process output data
            break;

        case XRCONN_STATE_ERROR:
            // process output data
            break;

        case XRCONN_STATE_BADREQ:
            // process output data
            break;

    }
#undef NEXT_STATE
}
static void on_handler_write(XrConnRef conn_ref, void* arg, int status)
{
    XR_TRACE("conn: %p arg: %p status: %d", conn_ref, arg, status);
    assert(conn_ref->handler_ref != NULL);
    IOBufferRef iobuf = XrHandler_function(conn_ref->req_msg_ref, conn_ref);
    if(iobuf != NULL) {
        XrConn_write(conn_ref, iobuf, &on_handler_write, arg);
    } else {
        close(conn_ref->fd);
        assert(false);
    }
}
static void on_handle_message(XrConnRef conn_ref, void* arg, int status)
{
    XR_TRACE("conn: %p arg: %p status: %d", conn_ref, arg, status);
    assert(conn_ref->handler_ref == NULL);
    IOBufferRef iobuf = XrHandler_function(conn_ref->req_msg_ref, conn_ref);
    assert(iobuf != NULL);
    assert(conn_ref->handler_ref != NULL);
    XrConn_write(conn_ref, iobuf, &on_handler_write, arg);
}
void listening_handler(XrWatcherRef wp, void *arg, uint64_t event)
{
    XrSocketWatcherRef sw = (XrSocketWatcherRef)wp;
    printf("listening_hander \n");
    struct sockaddr_in peername;
    unsigned int addr_length = (unsigned int) sizeof(peername);

    XrServerRef sref = arg;
    int sock2 = accept(sref->listening_socket_fd, (struct sockaddr *) &peername, &addr_length);
    if(sock2 <= 0) {
        LOG_FMT("%s %d", "Listener thread :: accept failed terminating sock2 : ", sock2);
    }
    XrSocketWatcherRef sw_ref = Xrsw_new(sref->reactor_ref, sock2);
    XrConnRef conn = XrConn_new(sock2, sw_ref, sref);
    MessageRef inmsg = Message_new();
    XrConn_read_msg(conn, inmsg, on_handle_message, (void*)conn);
    return;
    conn->state = XRCONN_STATE_RDINIT;
    uint64_t interest = EPOLLERR | EPOLLIN;
    Xrsw_register(sw_ref, &state_machine, conn, interest);
}

XrServerRef XrServer_new(int port)
{
    XrServerRef sref = (XrServerRef) eg_alloc(sizeof(XrServer));
    sref->port = port;
    return sref;
}

void XrServer_free(XrServerRef *sref)
{
    ASSERT_NOT_NULL(*sref);
    free(*sref);
    *sref = NULL;
}

void XrServer_listen(XrServerRef sref)
{
    ASSERT_NOT_NULL(sref)
    int port = sref->port;
    struct sockaddr_in peername;
    unsigned int addr_length = (unsigned int) sizeof(peername);
    sref->listening_socket_fd = create_listener_socket(port, "127.0.0.1");
    set_non_blocking(sref->listening_socket_fd);
    sref->reactor_ref = XrReactor_new();
    sref->listening_watcher_ref = Xrsw_new(sref->reactor_ref, sref->listening_socket_fd);
    XrSocketWatcherRef lw = sref->listening_watcher_ref;
    uint64_t interest = EPOLLIN | EPOLLERR;
    Xrsw_register(lw, listening_handler, sref, interest);
    XrReactor_run(sref->reactor_ref, -1);
}

void XrServer_terminate(XrServerRef this)
{
    close(this->listening_socket_fd);
}

