#ifndef c_http_async_types_h
#define c_http_async_types_h

#include <http_in_c/common/list.h>
#include <http_in_c/http/message.h>
//#include <http_in_c/async/conn_list.h>

typedef struct XrWorker_s XrWorker, *XrWorkerRef;
typedef struct AsyncServer_s AsyncServer, *AsyncServerRef;
typedef struct XrHandler_s XrHandler, *XrHandlerRef;
typedef struct TcpConn_s TcpConn, *TcpConnRef;

typedef ListRef TcpConnListRef;
typedef ListIter TcpConnListIter;

/**
 * Callback signatures for specific IO operations performed on a ConnRef
 */
typedef void (TcpConnReadCallback)(TcpConnRef conn, void* arg, int bytes_read, int status);
typedef void (*TcpConnReadMsgCallback)(TcpConnRef conn, void* arg, int status);
typedef void (*TcpConnWriteCallback)(TcpConnRef conn, void* arg, int status);

/**
 * Signature of function passed to handler. Called to signal handler is done
 * Must be "posted" with single argument set to the handlers TcpConnRef.
 */
typedef void (*HandlerDoneFunction)(void* conn_ref);

/**
 * HandlerFunction
 * TODO resolve defnition of xr handler function
 */
typedef void (*HandlerFunction)(MessageRef request, TcpConnRef conn_ref, HandlerDoneFunction done);
typedef void (*XrHandlerFunction)(MessageRef request, TcpConnRef conn_ref, HandlerDoneFunction done);

#endif