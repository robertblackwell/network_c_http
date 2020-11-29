#ifndef c_http_xr_xr_reader_h
#define c_http_xr_xr_reader_h
#include <c_http/list.h>
#include <c_http/rdsocket.h>
#include <c_http/message.h>
#include <c_http/ll_parser.h>
#include <c_http/buffer/iobuffer.h>
#include <c_http/socket_functions.h>

/**
 * Reader and its associated Reader_?? functions implement an object that can read and parse http messages from
 * either a real TCP socket or a DataSource* for testing.
 */

typedef struct XrReader_s
{
    ParserRef           m_parser;
    IOBufferRef         m_iobuffer;
    int                 m_socket;
    MessageRef          m_message_ref;
    int                 m_io_errno;
    int                 m_http_errno;
    char*               m_http_err_name;
    char*               m_http_err_description;

} XrReader, *XrReaderRef;

XrReaderRef XrReader_new(ParserRef parser, RdSocket rdsock);
void XrReader_init(XrReaderRef this, ParserRef parser, int sock);
void XrReader_destroy(XrReaderRef this);
void XrReader_free(XrReaderRef* this_ptr);

typedef enum XrReader_ReturnCode {
        XR_READER_EOM = 0,             // A message was returned
        XR_READER_EOF = -4,            // Other end closed but no message
        XR_READER_PARSE_ERROR = -1,   // An error in the format of the message was detected.
        XR_READER_IO_ERROR = -2,      // An IO error occurred.
        XR_READER_EAGAIN = -3
} XrReader_ReturnCode;

/**
 * Read a stream of http message from the m_readsocket data source/socket.
 *
 * This Reader object handles the processing of
 * -    taking data from a data source (such as a socket) into a buffers
 * -    pushing the buffer it into a Parser
 * -    handling the action on the various parser output states,
 *      which includes handling the situation where two messages overlap in a single buffer.
 *
 * Note: This function is designed to be used on both blocking and non-blocking byte sources. And hence
 * if the underlying source (socket) has no data available EAGAIN will be returned from the read
 * this will cause Xreader_read to return XR_READER_EAGAIN.
 * The XrReader_read function should be called again with exactly the same arguments when data is again
 * available on the byte source (socket). The XrReader object retains all required state data
 * to make the repeated call function correctly
 *
 * The reception of a full message is indicated by return of XR_READER_EOM. The message just read will
 * be available in this->message_ref. The XrReader object allocates the Message container.
 *
 * \param this              ReaderRef - the reader object
 * \return Reader_ReturnCode - Indicates whether successfull and if not nature if error.
 *                          TODO - on error the Reader struct will contain details of the error
 *                          for IO error it will hold the errno value related to the error
 *                          and for a parse error will hold the relevant http_errno value
 *                          together with char* pointers to the error name and description
 */
XrReader_ReturnCode XrReader_read(XrReaderRef this);

#endif