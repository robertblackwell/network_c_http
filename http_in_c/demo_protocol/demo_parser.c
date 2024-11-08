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
        DP_MessageComplete_CB on_new_message_callback,
        void* on_new_message_complete_ctx)
{
    DemoParserRef this = malloc(sizeof(DemoParser));
    RBL_SET_TAG(DemoParser_TAG, this)
    RBL_SET_END_TAG(DemoParser_TAG, this)
    this->parser_consume = (int(*)(ParserInterfaceRef, IOBufferRef))&DemoParser_consume;
    this->message_factory = (void*(*)())&demo_message_new;
    this->message_free = (void(*)(void*))&demo_message_free;
    this->m_state = STATE_IDLE;
    this->m_current_message_ptr = NULL;
    this->on_new_message_callback = on_new_message_callback;
    this->on_new_message_complete_ctx = on_new_message_complete_ctx;
    return this;
}
void DemoParser_free(DemoParserRef this)
{
    RBL_CHECK_TAG(DemoParser_TAG, this)
    RBL_CHECK_END_TAG(DemoParser_TAG, this)
    ASSERT_NOT_NULL(this);
    if(this->m_current_message_ptr) {
        demo_message_dispose(&(this->m_current_message_ptr));
    }
    free(this);
}
void DemoParser_dispose(DemoParserRef* this_p)
{
    RBL_CHECK_TAG(DemoParser_TAG, *this_p)
    RBL_CHECK_END_TAG(DemoParser_TAG, *this_p)
    ASSERT_NOT_NULL(*this_p);
    DemoParser_free(*this_p);
    *this_p = NULL;

}
DemoMessageRef DemoParser_current_message(DemoParserRef this)
{
    RBL_CHECK_TAG(DemoParser_TAG, this)
    RBL_CHECK_END_TAG(DemoParser_TAG, this)
    return this->m_current_message_ptr;
}
int DemoParser_append_bytes(DemoParserRef this, void *buffer, unsigned length)
{
    RBL_CHECK_TAG(DemoParser_TAG, this)
    RBL_CHECK_END_TAG(DemoParser_TAG, this)
    return 0;
}
void DemoParser_consume(DemoParserRef this, IOBufferRef iobuffer_ref)
{
    void* buf = IOBuffer_data(iobuffer_ref);
    int length = IOBuffer_data_len(iobuffer_ref);
    assert(length != 0);
    int error_code = 0;
    RBL_CHECK_TAG(DemoParser_TAG, this)
    RBL_CHECK_END_TAG(DemoParser_TAG, this)

    char* charbuf = (char*) buf;
    for(int i = 0; i < length; i++) {
        int ch2 = (int)(*(char*)IOBuffer_data(iobuffer_ref));
        IOBuffer_consume(iobuffer_ref, 1);
        int ch = (int)charbuf[i];
        assert(ch == ch2);
        switch (this->m_state) {
            case STATE_IDLE:
                if(this->m_current_message_ptr == NULL) {
                    this->m_current_message_ptr = demo_message_new();
                }
                if(ch == CH_STX) this->m_state = STATE_BODY;
                break;
            case STATE_BODY: {
                if (isprint(ch)) {
                    BufferChainRef bc = demo_message_get_body(this->m_current_message_ptr);
                    BufferChain_append(bc, &ch, 1);
                    IOBufferRef iobtmp = BufferChain_compact(bc);
                    const char* xx = IOBuffer_cstr(iobtmp);
                    IOBuffer_free(iobtmp);
                } else if (ch == CH_ETX) {
                    this->m_state = STATE_IDLE;
                    this->on_new_message_callback(this-> on_new_message_complete_ctx, this->m_current_message_ptr);
                    this->m_current_message_ptr = demo_message_new();
                    this->m_state = STATE_IDLE;
                } else {
                    this->m_state = STATE_ERROR_RECOVERY;
                }
                break;
            }
            case STATE_ERROR_RECOVERY:
                /**
                 * Come to this state after a parse error - search for SOH until end of buffer.
                 * If not found the terminate the read operation with an error
                 */
                if((ch == CH_STX) && (i < length - 1)) {
                    this->m_state = STATE_BODY;
                } else {
                    if(i == length - 1) {
                        RBL_LOG_FMT("DemmoParser_consume parse error \n");
                        demo_message_dispose(&(this->m_current_message_ptr));
                        this->m_current_message_ptr = demo_message_new();
                        return;
                    }
                }
                break;
            default:
                assert(false);
        }
    }
}

int DemoParser_get_errno(DemoParserRef this)
{
    RBL_CHECK_TAG(DemoParser_TAG, this)
    RBL_CHECK_END_TAG(DemoParser_TAG, this)
    return 0;
}
DemoParserError DemoParser_get_error(DemoParserRef this)
{
//    llhttp_errno_t x = llhttp_get_errno(this->m_llhttp_ptr);
//    char* n = (char*)llhttp_errno_name(x);
//    char* d = (char*)llhttp_errno_name(x);
    RBL_CHECK_TAG(DemoParser_TAG, this)
    RBL_CHECK_END_TAG(DemoParser_TAG, this)
    DemoParserError erst;
    erst.m_err_number = 1;
    erst.m_name = "";
    erst.m_description = "";
    return erst;
}

void DemoParser_initialize(DemoParserRef this)
{
    RBL_CHECK_TAG(DemoParser_TAG, this)
    RBL_CHECK_END_TAG(DemoParser_TAG, this)
    this->m_current_message_ptr = NULL;
}
