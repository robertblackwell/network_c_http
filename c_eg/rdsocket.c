#define _GNU_SOURCE

#include <c_eg/rdsocket.h>
#include <c_eg/alloc.h>
#include <c_eg/utils.h>
#include <c_eg/buffer/iobuffer.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

/**
 * These adapter functions are necessary since the read function that work on real sockets has a different calling signature than
 * the DataSource_read() function that work on DataSourceRef. These two wrappers uniformize the signature
 */
static int socket_read(void* sock_ctx, void* buffer, int len)
{
    int bytes_read = (int)read((int)sock_ctx, buffer, len);
    return bytes_read;
}

static int datasource_read(void* sock_ctx, void* buffer, int len)
{
    DataSourceRef dsref = (DataSourceRef)(sock_ctx);
    int bytes_read = DataSource_read((DataSourceRef)sock_ctx, buffer, len);
    return bytes_read;
}

RdSocket RealSocket(int socket)
{
    RdSocket rdsock;
    rdsock.read_f = &socket_read;
    rdsock.ctx = socket;
    return rdsock;
}
RdSocket DataSourceSocket(DataSourceRef dsref)
{
    RdSocket rdsock = {.ctx=(void*)dsref, .read_f=&datasource_read};
    return rdsock;
}
int RdSocket_read(RdSocketRef rdsock, void* buffer, int len)
{
    DataSourceRef tmp = (DataSourceRef)rdsock->ctx;
    return rdsock->read_f(rdsock->ctx, buffer, len);
}