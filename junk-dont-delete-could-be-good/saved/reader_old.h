#ifndef c_http_reader_h
#define c_http_reader_h
#include <http_in_c/common/list.h>
#include <http_in_c/common/http_parser/rdsocket.h>
#include <http_in_c/http/message.h>
#include <http_in_c/common/http_parser/ll_parser.h>
#include <http_in_c/common/iobuffer.h>
#include <http_in_c/common/socket_functions.h>

/**
 * Reader and its associated SyncReader_?? functions implement an object that can read and parse http messages from
 * either a real TCP socket or a DataSource* for testing.
 */

typedef struct SyncReader_s
{
    ParserRef           m_parser;
    IOBufferRef         m_iobuffer;
    RdSocket            m_rdsocket;
    int                 m_io_errno;
    int                 m_http_errno;
    char*               m_http_err_name;
    char*               m_http_err_description;

} Reader, *SyncReaderRef;

SyncReaderRef SyncReader_new(ParserRef parser, RdSocket rdsock);
void SyncReader_init(SyncReaderRef this, ParserRef parser, RdSocket rdsock);
void SyncReader_destroy(SyncReaderRef this);
void SyncReader_dispose(SyncReaderRef* this_ptr);

typedef enum SyncReader_ReturnCode {
        READER_OK = 0,             // A message was returned
        READER_PARSE_ERROR = -1,   // An error in the format of the message was detected.
        READER_IO_ERROR = -2,      // An IO error occurred.
} SyncReader_ReturnCode;

/**
 * Read a stream of http message from the m_readsocket data source/socket.
 *
 * This Reader object handles the processing of
 * -    taking data from a data source (such as a socket) in buffers
 * -    pushing the buffer it into a Parser
 * -    handling the action on the various parser output states,
 *      which includes handling the situation where two messages overlap in a single buffer.
 *
 *
 *
 * \param this              SyncReaderRef - the reader object
 * \param msgref_ptr        Variable into which a MessageRef value will be placed if a message is successfully read.
 * \return SyncReader_ReturnCode - Indicates whether successfull and if not nature if error.
 *                          TODO - on error the Reader struct will contain details of the error
 *                          for IO error it will hold the errno value related to the error
 *                          and for a parse error will hold the relevant http_errno value
 *                          together with char* pointers to the error name and description
 */
SyncReader_ReturnCode SyncReader_read(SyncReaderRef this, MessageRef* msgref_ptr);

#endif