#ifndef c_http_sync_reader_h
#define c_http_sync_reader_h
/**
 * @addtogroup group_reader Reader
 * @brief A module that reads complete http/1.x messages synchronously
 * @{
 */

#include <c_http/common/message.h>
#include <c_http/http_parser/ll_parser.h>
/**
 * Reader and its associated SyncReader_?? functions implement an object that can read and parse compete HTTP/1.X messages from
 * either a real TCP socket or a datasource_t* for testing.
 */
typedef struct SyncReader_s Reader, *SyncReaderRef;

/**
 * @brief Create a Reader from a socket fd
 * @param rdsock_fd int
 * @return SyncReaderRef
 */
SyncReaderRef SyncReader_new(int rdsock_fd, OnMessageCompleteHandler handler, void* handler_context);
//void SyncReader_init(SyncReaderRef this, RdSocket rdsock);
void SyncReader_destroy(SyncReaderRef this);
void SyncReader_dispose(SyncReaderRef* this_ptr);

typedef enum SyncReader_ReturnCode {
        READER_OK = 0,             // A message was returned
        READER_PARSE_ERROR = -1,   // An error in the format of the message was detected.
        READER_IO_ERROR = -2,      // An IO error occurred.
} SyncReader_ReturnCode;

/**
 * @brief Read a stream of http message from the m_readsocket data source/socket.
 *
 * This Reader object handles the processing of
 * -    taking data from a data source (such as a socket) in buffers
 * -    pushing the buffer it into a http_parser_t
 * -    handling the action on the various parser output states,
 *      which includes handling the situation where two messages overlap in a single buffer.
 *
 *
 *
 * @param this              SyncReaderRef - the reader object
 * @param msgref_ptr        Variable into which a MessageRef value will be placed if a message is successfully read.
 * @return SyncReader_ReturnCode - Indicates whether successfull and if not nature if error.
 *                          TODO - on error the Reader struct will contain details of the error
 *                          for IO error it will hold the errno value related to the error
 *                          and for a parse error will hold the relevant http_errno value
 *                          together with char* pointers to the error name and description
 */
SyncReader_ReturnCode SyncReader_read(SyncReaderRef this, MessageRef* msgref_ptr);
/** @} */
#endif