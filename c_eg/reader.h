#ifndef c_c_eg_reader_h
#define c_c_eg_reader_h
#include <c_eg/list.h>
#include <c_eg/rdsocket.h>
#include <c_eg/message.h>
#include <c_eg/parser.h>
#include <c_eg/buffer/iobuffer.h>
#include <c_eg/socket_functions.h>



typedef struct Rdr_s
{
    ParserRef           m_parser;
    IOBufferRef         m_iobuffer;
    RdSocket            m_rdsocket;

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
 * Read a http message.
 *
 * \param this
 * \param msgref_ptr
 * \return
 */
Rdr_ReturnCode Rdr_read(RdrRef this, MessageRef* msgref_ptr);

#endif