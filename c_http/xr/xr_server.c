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
#include <c_http/xr/xr_worker.h>
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

Reader_ReturnCode read_and_parse_some(XrConnectionRef this)
{
    IOBufferRef iobuf = this->io_buf_ref;
    MessageRef message_ptr = this->req_msg_ref;
    int bytes_read;
    //
    // handle nothing left in iobuffer
    // only read more if iobuffer is empty
    //
    if(IOBuffer_data_len(iobuf) == 0 ) {
        IOBuffer_reset(iobuf);
        void* buf = IOBuffer_space(iobuf);
        int len = IOBuffer_space_len(iobuf);
        bytes_read = read(this->fd, buf, len);
        if(bytes_read == 0) {
            if(!this->parser_ref->m_started) {
                // eof no message started - there will not be any more bytes to parse so cleanup and exit
                // return no error
                Message_free(&(message_ptr));
                this->req_msg_ref = NULL;
                return 0;
            }
            if(this->parser_ref->m_started && this->parser_ref->m_message_done) {
                // shld not get here
                assert(false);
            }
            if(this->parser_ref->m_started && !this->parser_ref->m_message_done) {
                // get here if otherend is signlaling eom with eof
                assert(true);
            }
        } else if(bytes_read > 0) {
            IOBuffer_commit(iobuf, bytes_read);
        } else if((bytes_read < 0)&&(errno == EAGAIN)) {
            // no data ready to read try again latter
            return READER_EAGAIN;
        } else {
            // have an io error
            int x = errno;
            Message_free(&(message_ptr));
            this->req_msg_ref = NULL;
            return READER_IO_ERROR;
        }
    } else {
        bytes_read = iobuf->buffer_remaining;
    }
    char *tmp = (char *) iobuf->buffer_ptr;
    char *tmp2 = (char *) iobuf->mem_p;
    ParserReturnValue ret = Parser_consume(this->parser_ref, (void *) IOBuffer_data(iobuf), IOBuffer_data_len(iobuf));
    int consumed = bytes_read - ret.bytes_remaining;
    IOBuffer_consume(iobuf, consumed);
    int tmp_remaining = iobuf->buffer_remaining;
    switch(ret.return_code) {
        case ParserRC_error:
            ///
            /// got a parse error - need some way to signal the caller so can send reply of bad message
            ///
            Message_free(&message_ptr);
            this->req_msg_ref = NULL;
            return READER_PARSE_ERROR;
            break;
        case ParserRC_end_of_data:
            break;
        case ParserRC_end_of_header:
            break;
        case ParserRC_end_of_message:
            // return ok
            return READER_OK;
            break;
    }
}

static void state_machine(XrSocketWatcherRef sw, void *arg, uint64_t event)
{
    XrConnectionRef conn_ref = arg;
    XrReactorRef reactor_ref = conn_ref->server_ref->reactor_ref;
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
        case XRCONN_STATE_READ: {
            assert(conn_ref->req_msg_ref != NULL);
            assert(conn_ref->parser_ref != NULL);
            Reader_ReturnCode rc = read_and_parse_some(conn_ref);
            // have to decide what next
            printf("Read %d \n", rc);
            if(conn_ref->parser_ref->m_message_done) {
                conn_ref->state = XRCONN_STATE_HANDLE;
                // turn off read events
                Xrsw_change_watch(sw, &state_machine, arg, 0);
                XrReactor_post(reactor_ref, sw, &state_machine, arg);
            }
        }
            break;

        case XRCONN_STATE_WRITE:
            // process output data
            break;

        case XRCONN_STATE_HANDLE:assert(false);
            break;

        case XRCONN_STATE_UNINIT:assert(false);
            break;
    }
}

void listening_handler(XrSocketWatcherRef sw, void *arg, uint64_t event)
{
    printf("listening_hander \n");
    struct sockaddr_in peername;
    unsigned int addr_length = (unsigned int) sizeof(peername);

    XrServerRef sref = arg;
    int sock2 = accept(sref->listening_socket_fd, (struct sockaddr *) &peername, &addr_length);
    if(sock2 <= 0) {
        LOG_FMT("%s %d", "Listener thread :: accept failed terminating sock2 : ", sock2);
    }
    XrSocketWatcherRef sw_ref = Xrsw_new(sref->reactor_ref, sock2);
    XrConnectionRef conn = XrConnection_new(sock2, sw_ref, sref);
    conn->state = XRCONN_STATE_READ;
    uint64_t interest = EPOLLERR | EPOLLIN;
    Xrsw_register(sw_ref, &state_machine, conn, interest);
}

XrServerRef XrServer_new(int port, XrHandlerFunction handler)
{
    XrServerRef sref = (XrServerRef) eg_alloc(sizeof(XrServer));
    sref->port = port;
    sref->handler = handler;
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

