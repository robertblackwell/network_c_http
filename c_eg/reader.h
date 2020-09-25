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
 * either a real TCP socket or a DataSourceRef for testing.
 */

typedef struct Rdr_s
{
    ParserRef           m_parser;
    IOBufferRef         m_iobuffer;
    RdSocket            m_rdsocket;
    int                 m_io_errno;
    int                 m_http_errno;
    char*               m_http_err_name;
    char*               m_http_err_description;

} Rdr, *RdrRef;

RdrRef Rdr_new(ParserRef parser, RdSocket rdsock);
void Rdr_init(RdrRef this, ParserRef parser, RdSocket rdsock);
void Rdr_destroy(RdrRef this);
void Rdr_free(RdrRef* this_ptr);

typedef enum Rdr_ReturnCode {
        RDR_OK = 0,
        RDR_PARSE_ERROR = -1,
        RDR_IO_ERROR = -2,
} Rdr_ReturnCode;

/**
 * Read a http message from the m_readsocket data source/socket
 *
 * \param this        RdrRef - the reader object
 * \param msgref_ptr  Variable into which a MessageRef value will be placed if a message is successfully read.
 * \return Rdr_ReturnCode - indicates whether successfull and if not nature if error/
 *                          TODO - on error the Rdr struct will contain details of the error
 *                          for IO error it will hold the errno value related to the error
 *                          and for a parse error will hold the relevant http_errno value
 *                          together with chr* pointers to the error name and description
 */
Rdr_ReturnCode Rdr_read(RdrRef this, MessageRef* msgref_ptr);

#endif