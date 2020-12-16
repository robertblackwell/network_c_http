#ifndef c_http_reader_h
#define c_http_reader_h
/**
 * @addtogroup group_reader Reader
 * @brief A module that reads complete http/1.x messages synchronously
 * @{
 */

#include <c_http/details/rdsocket.h>
#include <c_http/api/message.h>

/**
 * Reader and its associated Reader_?? functions implement an object that can read and parse compete HTTP/1.X messages from
 * either a real TCP socket or a DataSource* for testing.
 */
typedef struct Reader_s Reader, *ReaderRef;

/**
 * @brief Create a Reader from a socket fd
 * @param rdsock_fd int
 * @return ReaderRef
 */
ReaderRef Reader_new(int rdsock_fd);
//void Reader_init(ReaderRef this, RdSocket rdsock);
void Reader_destroy(ReaderRef this);
void Reader_dispose(ReaderRef* this_ptr);

typedef enum Reader_ReturnCode {
        READER_OK = 0,             // A message was returned
        READER_PARSE_ERROR = -1,   // An error in the format of the message was detected.
        READER_IO_ERROR = -2,      // An IO error occurred.
} Reader_ReturnCode;

/**
 * @brief Read a stream of http message from the m_readsocket data source/socket.
 *
 * This Reader object handles the processing of
 * -    taking data from a data source (such as a socket) in buffers
 * -    pushing the buffer it into a Parser
 * -    handling the action on the various parser output states,
 *      which includes handling the situation where two messages overlap in a single buffer.
 *
 *
 *
 * @param this              ReaderRef - the reader object
 * @param msgref_ptr        Variable into which a MessageRef value will be placed if a message is successfully read.
 * @return Reader_ReturnCode - Indicates whether successfull and if not nature if error.
 *                          TODO - on error the Reader struct will contain details of the error
 *                          for IO error it will hold the errno value related to the error
 *                          and for a parse error will hold the relevant http_errno value
 *                          together with char* pointers to the error name and description
 */
Reader_ReturnCode Reader_read(ReaderRef this, MessageRef* msgref_ptr);
/** @} */
#endif