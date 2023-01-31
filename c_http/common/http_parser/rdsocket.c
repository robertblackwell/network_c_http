#define _GNU_SOURCE

#include <c_http/common/http_parser/rdsocket.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>

/**
 * These adapter functions are necessary since the read function that work on real sockets has a different calling signature than
 * the DataSource_read() function that work on DataSource*. These two wrappers uniformize the signature
 */
 /**
  *
  * \param sock_ctx An OS sock file descriptor(int) cast to a void*. Needs to be cast back before use
  * \param buffer   void* pointer to buffer
  * \param len      int   len of available space in the buffer
  * \return         int number of bytes read on success,
  *                 0 generally means the socket was closed at the other end,
  *                 negative means io error
  *                 TODO implement EAGAIN testing
  */
static int socket_read(RdSocket* rdsock_ref, void* buffer, int len)
{
    void* sock_ctx = rdsock_ref->ctx;
    int sock_fd = (int)(long)sock_ctx;
    int bytes_read = (int)read(sock_fd, buffer, len);
    rdsock_ref->m_errno = errno;
    return bytes_read;
}

static int datasource_read(RdSocket* rdsock_ref, void* buffer, int len)
{
    void* sock_ctx = rdsock_ref->ctx;
    DataSource* dsref = (DataSource*)(sock_ctx);
    int bytes_read = DataSource_read((DataSource*)sock_ctx, buffer, len);
    rdsock_ref->m_errno = dsref->m_errno;
    return bytes_read;
}

RdSocket RealSocket(int sock)
{
    RdSocket rdsock;
    rdsock.read_f = (ReadFunc) &socket_read;
    /**
     * this next cast could conceivably be a problem is sizeof(int) > sizeof(void*) - so test
     */
    static_assert(sizeof(int) <= sizeof(void*));
    rdsock.ctx = (void*)(long)sock;
    return rdsock;
}
RdSocket DataSourceSocket(DataSource* dsref)
{
    RdSocket rdsock = {.ctx=(void*)dsref, .read_f=(ReadFunc) &datasource_read};
    return rdsock;
}
int RdSocket_read(RdSocket* rdsock, void* buffer, int len)
{
    DataSource* tmp = (DataSource*)rdsock->ctx;
    int bytes_read = rdsock->read_f(rdsock, buffer, len);
    return bytes_read;
}