#ifndef c_c_eg_rdsocket_h
#define c_c_eg_rdsocket_h
#include <c_eg/list.h>
#include <c_eg/message.h>
#include <c_eg/parser.h>
#include <c_eg/buffer/iobuffer.h>
#include <c_eg/socket_functions.h>
#include <c_eg/datasource.h>

/**
 *
 * Want to parametrize Reader type with the function that actually does the reading
 * so that in test_parser I can substitute a mock socket reader.
 * There are two cases :
 *  Real socket
 *      function is read()
 *      ctx is int(socket_handle_t) socket
 *  Testing DataSource
 *      function is DataSource_read()
 *      ctx is DataSourceRef
 *
 *  RdSocket is a type that generalizes both cases and provides a uniform interface
 *  for reading data from these two sources.
 */


typedef int(*ReadFunc)(void* ctx, void* bufptr, int buflen);
typedef struct RdSocket_s {
    ReadFunc read_f;
    void* ctx;

} RdSocket, *RdSocketRef;

/**
 * create a RdSocket from a real tcp socket
 * \return RdSocket by value
 */
RdSocket RealSocket(int socket);
/**
 * Creates a RdSocket from as DataSourceRef - which is a pointer -
 * the instance holds the DataSourceRef without ownership and does not free it.
 *
 * WARNING: ensure the DataSourceRef remains valid for the duration of use of the RdSocket. Oh for a shared_ptr
 *
 * \param datasource DataSourceRef
 * \return RdSocket By value
 */
RdSocket DataSourceSocket(DataSourceRef datasource);
/**
 * Call the read function of a RdSocket - obviously - works for both types of read sockets
 *
 * \param rdsock_ref RdSocket* - note: since there is no RdSocket_new will have to call this function in
 *                                  the form int bytes_read = RdSocket_read(&(rdsock), buffer, len)
 *                                  note the &
 * \param buffer     void*
 * \param buffer_len int
 * \return number of bytes read or error condition.
 *          Errors:
 *              0 means other end of source closed.
 *              < 0 means IO error. For real sockets errno will give details. For DataSource version only -1
 */
int RdSocket_read(RdSocketRef rdsock_ref, void* buffer, int buffer_len);


#endif