#ifndef c_c_eg_reader_h
#define c_c_eg_reader_h
#include <c_eg/list.h>
#include <c_eg/rdsocket.h>
#include <c_eg/message.h>
#include <c_eg/parser.h>
#include <c_eg/buffer/iobuffer.h>
#include <c_eg/socket_functions.h>

/**
 * Rdr and its associated Rdr_?? functions implement an object that can read and parse http messages from
 * either a real TCP socket or a DataSource* for testing.
 */

typedef struct Rdr_s
{
    Parser*           m_parser;
    IOBufferRef         m_iobuffer;
    RdSocket            m_rdsocket;
    int                 m_io_errno;
    int                 m_http_errno;
    char*               m_http_err_name;
    char*               m_http_err_description;

} Rdr;

Rdr* Rdr_new(Parser* parser, RdSocket rdsock);
void Rdr_init(Rdr* this, Parser* parser, RdSocket rdsock);
void Rdr_destroy(Rdr* this);
void Rdr_free(Rdr** this_ptr);

typedef enum Rdr_ReturnCode {
        RDR_OK = 0,             // A message was returned
        RDR_PARSE_ERROR = -1,   // An error in the format of the message was detected.
        RDR_IO_ERROR = -2,      // An IO error occurred.
} Rdr_ReturnCode;

/**
 * Read a stream of http message from the m_readsocket data source/socket.
 *
 * This Rdr object handles the processing of
 * -    taking data from a data source (such as a socket) in buffers
 * -    pushing the buffer it into a Parser
 * -    handling the action on the various parser output states,
 *      which includes handling the situation where two messages overlap in a single buffer.
 *
 *
 *
 * \param this              Rdr* - the reader object
 * \param msgref_ptr        Variable into which a Message* value will be placed if a message is successfully read.
 * \return Rdr_ReturnCode - Indicates whether successfull and if not nature if error.
 *                          TODO - on error the Rdr struct will contain details of the error
 *                          for IO error it will hold the errno value related to the error
 *                          and for a parse error will hold the relevant http_errno value
 *                          together with char* pointers to the error name and description
 */
Rdr_ReturnCode Rdr_read(Rdr* this, Message** msgref_ptr);

#endif