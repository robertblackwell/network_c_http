#ifndef c_http_rdsocket_h
#define c_http_rdsocket_h
#include <c_http/common/list.h>
#include <c_http/common/message.h>
#include <c_http/common/iobuffer.h>
#include <c_http/socket_functions.h>
#include <c_http/http_parser/datasource.h>

/**
 * @addtoggroup group_rdsocket
 * Want to parametrize Reader type with the function that actually does the reading
 * so that in test_parser I can substitute a mock socket reader.
 * There are two cases :
 *  Real socket
 *      function is read()
 *      ctx is int(socket_handle_t) socket
 *  Testing datasource_t
 *      function is datasource_read_some()
 *      ctx is datasource_t*
 *
 *  RdSocket is a type that generalizes both cases and provides a uniform interface
 *  for reading data from these two sources.
 * @{
 */


typedef int(*ReadFunc)(void* ctx, void* bufptr, int buflen);
typedef struct RdSocket_s {
    ReadFunc read_f;
    void* ctx;
    int   m_errno;

} RdSocket;

/**
 * create a RdSocket from a real tcp socket
 * \return RdSocket by value
 */
RdSocket RealSocket(int socket);

/**
 * Creates a RdSocket from as datasource_t* - which is a pointer -
 * the instance holds the datasource_t* without ownership and does not free it.
 *
 * WARNING: ensure the datasource_t* remains valid for the duration of use of the RdSocket. Oh for a shared_ptr
 *
 * \param datasource datasource_t*
 * \return RdSocket By value
 */
RdSocket DataSourceSocket(datasource_t* datasource);

/**
 * This function is the uniform interface for reading data from a RdSocket regardless of its underlying data source.
 *
 * It calls the rdsock_ref->read_f function to access the type specific read function
 *
 * \param rdsock_ref RdSocket* - note: since there is no RdSocket_new will have to call this function in
 *                                  the form
 *                                  int bytes_read = RdSocket_read(&(rdsock), buffer, len)
 *                                  note the &
 * \param buffer     void*
 * \param buffer_len int
 * \return number of bytes read or error condition.
 *          Errors:
 *              0 means other end of source closed.
 *              < 0 means IO error. for real sockets the system errno value will be
 *              captured and stored in rdsock_ref->m_errno
 *              For datasource_t based RdSocket a simulated errno will be derived from the datasource_t instance
 */
int RdSocket_read(RdSocket* rdsock_ref, void* buffer, int buffer_len);

/**
 * Gets the errno from a RdSocket*
 * \param rdsock_ref RdSocketRef
 * \return           int errno from the most recent read if that read experienced an io error
 */
int RdSocket_errno(RdSocket* rdsock_ref);

/** @} */

#endif