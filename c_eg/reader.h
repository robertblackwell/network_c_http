#ifndef c_c_eg_reader_h
#define c_c_eg_reader_h
#include <c_eg/list.h>
#include <c_eg/message.h>
#include <c_eg/parser.h>
#include <c_eg/buffer/iobuffer.h>
#include <c_eg/socket_functions.h>

typedef struct Rdr_s
{
    ParserRef           m_parser;
    int                 m_socket;
    IOBufferRef         m_iobuffer;
} Rdr, *RdrRef;

RdrRef Rdr_new(ParserRef parser, int socket);
void Rdr_init(RdrRef this, ParserRef parser, int socket);
void Rdr_destroy(RdrRef this);
void Rdr_free(RdrRef* this_ptr);
MessageRef Rdr_read(RdrRef this);

#endif