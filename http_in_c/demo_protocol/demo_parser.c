#include <http_in_c/demo_protocol/demo_parser.h>
#include <http_in_c/common/utils.h>
#include <ctype.h>
#include <rbl/logger.h>

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
    STATE_EOT_WAIT,
    STATE_ERROR_RECOVERY
};
#define CH_SOH 0x01
#define CH_STX 0x02
#define CH_ETX 0x03
#define CH_EOT 0x04
#define is_ascii(ch) (isascii(ch))

#define PARSE_ERROR_CB(THIS, BYTES, ERRC) do{ \
    IOBuffer_consume(iobuffer_ref, BYTES);                                        \
    DemoParserPrivateReturnValue rv = {.error_code = (ERRC)}; \
    THIS->on_read_parser_error_cb(THIS->on_read_ctx, ERRC##_message);                                        \
    /*return rv;*/               \
}while(0);

#define SET_IS_REQUEST(THIS, V) (DemoMessage_set_is_request(THIS->m_current_message_ptr, V)
#define ADD_TO_BODY(THIS, CH)   ()
#define FINALIZE_BODY(THIS)

void DemoParser_initialize(DemoParserRef this);

DemoParserRef DemoParser_new(
        DP_MessageComple_CB on_message_complete_cb,
        void* on_read_ctx)
{
    DemoParserRef this = malloc(sizeof(DemoParser));
    SET_TAG(DemoParser_TAG, this)
    this->parser_consume = (int(*)(ParserInterfaceRef, IOBufferRef))&DemoParser_consume;
    this->message_factory = (void*(*)())&demo_message_new;
    this->message_free = (void(*)(void*))&demo_message_free;
    this->m_state = STATE_IDLE;
    this->m_current_message_ptr = NULL;
    this->on_message_complete = on_message_complete_cb;
    this->on_read_ctx = on_read_ctx;
    return this;
}
void DemoParser_free(DemoParserRef this)
{
    CHECK_TAG(DemoParser_TAG, this)
    ASSERT_NOT_NULL(this);
    if(this->m_current_message_ptr) {
        demo_message_dispose(&(this->m_current_message_ptr));
    }
    free(this);
}
void DemoParser_dispose(DemoParserRef* this_p)
{
    CHECK_TAG(DemoParser_TAG, *this_p)
    ASSERT_NOT_NULL(*this_p);
    DemoParser_free(*this_p);
    *this_p = NULL;

}
DemoMessageRef DemoParser_current_message(DemoParserRef this)
{
    CHECK_TAG(DemoParser_TAG, this)
    return this->m_current_message_ptr;
}
int DemoParser_append_bytes(DemoParserRef this, void *buffer, unsigned length)
{
    CHECK_TAG(DemoParser_TAG, this)
    return 0;
}
/**
 * Comsumes some or all of the data given by buf and length and either partially or fully parses a DemoMessage.
 * The possible outcomes are:
 *   1   consumes all of the data annd only parses a partial message
 *   2   comsumes only some of the data and only parses part of a message - THIS SHOULD NEVER HAPPEN
 *   3   consume all of the data and complete the parsing of a message.
 *          IN which case the message is in m_current_message_ptr
 *          and the bytes_consumed == length or the input IOBUffer is now empty
 *   4   consume only some of the data and completes the parsing of a message.
 *          this means that the input buffer had the tail of one message and the start of another
 *          the completed message is in m_current_message_ptr
 *          bytes_consumed < length
 *          the input IOBUffer is NOT empty
 *          the input IOBuffer MUST be presented again to the parser because there is unprocessed data in it
 *    3. error - a parse error was detected and there is data left in the buffer unprocessed
 *          error_code will give details of the error
 *          the buffer must be presented again
 *    4. error - a parse error was detected and the is NO data left unprocessed in the buffer
 *          error_code will give details of the error
 *          the buffer should NOT be presented again
 */
DemoParserErrCode DemoParser_consume(DemoParserRef this, IOBufferRef iobuffer_ref)
{
    void* buf = IOBuffer_data(iobuffer_ref);
    int length = IOBuffer_data_len(iobuffer_ref);
    int error_code = 0;
    CHECK_TAG(DemoParser_TAG, this)

    char* charbuf = (char*) buf;
    for(int i = 0; i < length; i++) {
        int ch = (int)charbuf[i];
        switch (this->m_state) {
            case STATE_IDLE:
                if(this->m_current_message_ptr == NULL) {
                    this->m_current_message_ptr = demo_message_new();
                }
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
                        this->m_state = STATE_ERROR_RECOVERY;
                        error_code = DemoParserErr_invalid_opcode;
                        break;
                }
                break;
            case STATE_STX_WAIT: {
                if (ch == CH_STX) {
                    this->m_state = STATE_BODY;
                } else {
                    this->m_state = STATE_ERROR_RECOVERY;
                    error_code = DemoParserErr_expected_stx;
                }
                break;
            }
            case STATE_BODY: {
                if (isprint(ch)) {
                    BufferChainRef bc = demo_message_get_body(this->m_current_message_ptr);
                    BufferChain_append(bc, &ch, 1);
                    IOBufferRef iobtmp = BufferChain_compact(bc);
                    const char* xx = IOBuffer_cstr(iobtmp);
                    IOBuffer_free(iobtmp);
                } else if (ch == CH_ETX) {
                    this->m_state = STATE_LRC;
//                    finalize_body(this);
                } else {
                    this->m_state = STATE_ERROR_RECOVERY;
                    error_code = DemoParserErr_expected_ascii;
                }
                break;
            }
            case STATE_LRC:
                demo_message_set_lrc(this->m_current_message_ptr, ch);
                this->m_state = STATE_IDLE;
                LOG_FMT("DemmoParser_consume got a message will call new message handler \n");
                this->on_message_complete(this->on_read_ctx, this->m_current_message_ptr, 0);
                this->m_current_message_ptr = demo_message_new();
                IOBuffer_consume(iobuffer_ref, i+1);
                return 0;
                break;
            case STATE_EOT_WAIT:
                break;
            case STATE_ERROR_RECOVERY:
                /**
                 * Come to this state after a parse error - search for SOH until end of buffer.
                 * If not found the terminate the read operation with an error
                 */
                if((ch == CH_SOH) && (i < length - 1)) {
                    this->m_state = STATE_OPCODE;
                } else {
                    if(i == length - 1) {
                        LOG_FMT("DemmoParser_consume parse error \n");
                        this->on_message_complete(this->on_read_ctx, NULL, DemoParserErr_expected_stx);
                        demo_message_dispose(&(this->m_current_message_ptr));
                        this->m_current_message_ptr = demo_message_new();
                        IOBuffer_consume(iobuffer_ref, i + 1);
                        return DemoParserErr_expected_stx;
                    }
                }
                break;
            default:
                assert(false);
        }
    }
    IOBuffer_consume(iobuffer_ref, length);
    return 0;
}

int DemoParser_get_errno(DemoParserRef this)
{
    CHECK_TAG(DemoParser_TAG, this)
    return 0;
}
DemoParserError DemoParser_get_error(DemoParserRef this)
{
//    llhttp_errno_t x = llhttp_get_errno(this->m_llhttp_ptr);
//    char* n = (char*)llhttp_errno_name(x);
//    char* d = (char*)llhttp_errno_name(x);
    CHECK_TAG(DemoParser_TAG, this)
    DemoParserError erst;
    erst.m_err_number = 1;
    erst.m_name = "";
    erst.m_description = "";
    return erst;
}

void DemoParser_initialize(DemoParserRef this)
{
    CHECK_TAG(DemoParser_TAG, this)
    this->m_current_message_ptr = NULL;
}
