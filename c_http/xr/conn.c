#include <c_http/xr/conn.h>
#include <unistd.h>
#include <stdlib.h>

XrConnectionRef XrConnection_new(int fd, XrSocketWatcherRef socket_w, XrServerRef server_ref)
{
    XrConnectionRef tmp = malloc(sizeof(XrConnection));
    tmp->fd = fd;
    tmp->sock_watcher_ref = socket_w;
    tmp->server_ref = server_ref;
    tmp->state = XRCONN_STATE_UNINIT;
    tmp->io_buf_ref = IOBuffer_new ();
    tmp->req_msg_ref = Message_new ();
    tmp->response_buf_ref = Cbuffer_new ();
    tmp->parser_ref = Parser_new();
    Parser_begin(tmp->parser_ref, tmp->req_msg_ref);
    return tmp;
}

void XrConnection_free(XrConnectionRef this)
{
    Xrsw_free (this->sock_watcher_ref);
    free(this);
}
