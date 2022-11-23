#include <c_http/demo_protocol/demo_parser.h>
#include <c_http/common/utils.h>
#include <ctype.h>
/**
 * @addtogroup group_parser
 * @{
 */
//
// simple ascii protocol.
// Frame is :
//  SOH     1 byte  equal to soh character 0x01
//  OPCODE  1 byte 'R' or 'Q'
//  STX     1 byte 0x02
//  length  4 bytes ascii encoded length makes largest body 9999
//  body    'length' bytes all ascii
//  ETX     1 byte equal to etx 0x03
//  LRC
//  EOT     1 byte 0x04

enum State {
    STATE_IDLE,
    STATE_SOH_WAIT,
    STATE_OPCODE,
    STATE_STX_WAIT,
    STATE_BODY,
    STATE_EXT_WAIT,
    STATE_LRC,
    STATE_EOT_WAIT
};
#define CH_SOH 0x01
#define CH_STX 0x02
#define CH_ETX 0x03
#define CH_EOT 0x04
#define is_ascii(ch) (isascii(ch))

#define ERROR_RETURN(THIS, BYTES, ERRC) do{ \
    DemoParserReturnValue rv = {.eom_flag = false, .error_code = (ERRC), .bytes_consumed = (BYTES)}; \
    return rv;               \
}while(0);

#define SET_IS_REQUEST(THIS, V) (DemoMessage_set_is_request(THIS->m_current_message_ptr, V)
#define ADD_TO_BODY(THIS, CH)   ()
#define FINALIZE_BODY(THIS)

void DemoParser_initialize(DemoParserRef this);

DemoParserRef DemoParser_new()
{
    DemoParserRef this = malloc(sizeof(DemoParser));
    if(this == NULL)
        return NULL;
    this->m_message_done = false;
    return this;
}

void DemoParser_dispose(DemoParserRef* this_p)
{
    ASSERT_NOT_NULL(*this_p);
    DemoParserRef this= *this_p;
    free(this);
    *this_p = NULL;

}
DemoMessageRef DemoParser_current_message(DemoParserRef this)
{
    return this->m_current_message_ptr;
}
int DemoParser_append_bytes(DemoParserRef this, void *buffer, unsigned length)
{
    return 0;
}
void DemoParser_begin(DemoParserRef this, DemoMessageRef message_ptr)
{
    DemoParser_initialize(this);
    this->m_current_message_ptr = message_ptr;
    this->m_state = STATE_IDLE;
}

DemoParserReturnValue DemoParser_consume(DemoParserRef this, const void* buf, int length)
{
    char* charbuf = (char*) buf;
    for(int i = 0; i < length; i++) {
        int ch = charbuf[i];
        switch (this->m_state) {
            case STATE_IDLE:
                if(ch == CH_SOH) this->m_state = STATE_OPCODE;
                break;
            case STATE_OPCODE:
                switch(ch) {
                    case 'R':
                    case 'Q': {
                        demo_message_set_is_request(this->m_current_message_ptr, (ch == 'Q'));
                        this->m_state = STATE_STX_WAIT;
                        break;
                    }
                    default:
                        ERROR_RETURN(this, i, DemoParserErr_invalid_opcode)
                        break;
                }
                break;
            case STATE_STX_WAIT: {
                if (ch == CH_STX) {
                    this->m_state = STATE_BODY;
                } else {
                    this->m_state = STATE_IDLE;
                    ERROR_RETURN(this, i, DemoParserErr_expected_stx);
                }
                break;
            }
            case STATE_BODY: {
                if (isprint(ch)) {
                    BufferChainRef bc = demo_message_get_body(this->m_current_message_ptr);
                    BufferChain_append(bc, &ch, 1);
                    IOBufferRef iobtmp = BufferChain_compact(bc);
                    const char* xx = IOBuffer_cstr(iobtmp);
                } else if (ch == CH_ETX) {
                    this->m_state = STATE_LRC;
//                    finalize_body(this);
                } else {
                    this->m_state == STATE_IDLE;
                    ERROR_RETURN(this, i, DemoParserErr_expected_ascii);
                }
                break;
            }
            case STATE_LRC:
                demo_message_set_lrc(this->m_current_message_ptr, ch);
                this->m_state = STATE_IDLE;
                DemoParserReturnValue r = {.eom_flag = true, .error_code = 0, .bytes_consumed = i + 1};
                return r;
                break;
            case STATE_EOT_WAIT:
                break;
            default:
                assert(false);
        }
    }
    DemoParserReturnValue r = {.eom_flag = false, .bytes_consumed = length, .error_code = 0};
    return r;
}

int DemoParser_get_errno(DemoParserRef this)
{
    return 0;
}
DemoParserError DemoParser_get_error(DemoParserRef this)
{
//    llhttp_errno_t x = llhttp_get_errno(this->m_llhttp_ptr);
//    char* n = (char*)llhttp_errno_name(x);
//    char* d = (char*)llhttp_errno_name(x);
    DemoParserError erst;
    erst.m_err_number = 1;
    erst.m_name = "";
    erst.m_description = "";
    return erst;
}

void DemoParser_initialize(DemoParserRef this)
{
    this->m_started = false;
    this->m_message_done = false;
    this->m_current_message_ptr = NULL;
}
