

#include <http_in_c/saved/rdsocket.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>

/**
 * These adapter functions are necessary since the read function that work on real sockets has a different calling signature than
 * the datasource_read_some() function that work on datasource_t*. These two wrappers uniformize the signature
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
    datasource_t* dsref = (datasource_t*)(sock_ctx);
    int bytes_read = datasource_read_some((datasource_t *) sock_ctx, buffer, len);
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
RdSocket DataSourceSocket(datasource_t* dsref)
{
    RdSocket rdsock = {.ctx=(void*)dsref, .read_f=(ReadFunc) &datasource_read};
    return rdsock;
}
int RdSocket_read(RdSocket* rdsock, void* buffer, int len)
{
    datasource_t* tmp = (datasource_t*)rdsock->ctx;
    int bytes_read = rdsock->read_f(rdsock, buffer, len);
    return bytes_read;
}