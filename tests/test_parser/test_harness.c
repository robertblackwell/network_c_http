

#include "test_harness.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <http_in_c/common/alloc.h>
#include <http_in_c/common/utils.h>
#include <http_in_c/common/iobuffer.h>

/* Test input - is a list of blocks of text and some context so that these blocks can be
 * "read" and "processed" one at a time*/
void test_input_init(test_input_t* this, const char* description, const char *blocks[])
{
    this->description = description;
    this->block_count = 0;
    this->blocks = blocks;
}
const char* test_input_next(test_input_t* this)
{
    const char* block = this->blocks[this->block_count];
    this->block_count++;
    return block;
}
bool test_input_finished(test_input_t* this)
{
    return (this->blocks[this->block_count] == NULL);
}
int test_input_read_some(test_input_t* this, void* buffer, int length)
{
    const char* b0 = this->blocks[0];
    const char* b1 = this->blocks[1];
    const char* b2 = this->blocks[2];
    const char* b3 = this->blocks[3];
    const char* b4 = this->blocks[4];
    const char* block = this->blocks[this->block_count];
    if (block == NULL) {
        return 0;
    } else if (strcmp(block, "error") == 0) {
        this->errno = -99;
        return -1;
    } else {
        this->block_count++;
        int block_len = (int)strlen(block);
        assert(block_len < length);
        memcpy((void*)buffer, block, block_len);
        return block_len;
    }
}
test_output_r test_output_new(MessageRef msg, int rc)
{
    test_output_r rdref = eg_alloc(sizeof(test_output_t));
    rdref->message = msg;
    rdref->rc = rc;
}
void test_output_dispose(test_output_r* this_ptr)
{
    test_output_r this = *this_ptr;
    if(this->message != NULL)
        Message_dispose(&(this->message));
}

void parser_test_init(parser_test_t* this, const char* description, const char* lines[], verify_function_t verify_func)
{
    ASSERT_NOT_NULL(this);
    test_input_init(&this->test_input, description, lines);
    this->m_verify_func = verify_func;
    this->m_results = List_new(NULL);
}
parser_test_t* parser_test_new(const char* description, const char* blocks[], verify_function_t vf)
{
    parser_test_t* p = malloc(sizeof(parser_test_t));
    parser_test_init(p, description, blocks, vf);
    return p;
}
void parser_test_destroy(parser_test_t* this)
{
    ASSERT_NOT_NULL(this);
}
llhttp_errno_t on_message_handler(http_parser_t* parser_ref, MessageRef msg_ref)
{
    parser_test_t* ptest = parser_ref->handler_context;
    test_output_r r = test_output_new(msg_ref, 0);
    List_add_back(ptest->m_results, r);
    return HPE_OK;
}

int parser_test_run(parser_test_t* this)
{
    IOBufferRef iobuf_ref = IOBuffer_new_with_capacity(256);
    test_input_t* ds_ptr = &this->test_input;

    http_parser_t* pref = http_parser_new(on_message_handler, (void*) this);
    while(1) {
        llhttp_errno_t rc;
//        char*data = test_input_next(ds_ptr);
//        int len = (data != NULL ) ? strlen(data): 0;
        char buffer[1000];
        char* b = (char*) &buffer;
        int length = 900;
        int status = test_input_read_some(ds_ptr, &buffer, length);
        if(status > 0) {
            rc = http_parser_consume(pref, (void *) buffer, status);
            if(rc != HPE_OK) {
                printf("WPT_run status > 0 rc: %d\n", rc);
                List_add_back(this->m_results, test_output_new(NULL, rc));
                break;
            }
        } else if(status == 0) {
            rc = http_parser_consume(pref, NULL, 0);
            if(rc != HPE_OK) {
                printf("WPT_run status == 0 rc: %d\n", rc);
                List_add_back(this->m_results, test_output_new(NULL, rc));
            }
            break;
        } else {
            printf("WPT_run status < 0 io error rc: %d\n", HPE_USER);
            List_add_back(this->m_results, test_output_new(NULL, HPE_USER));
            break;
        }
    }
    int r =this->m_verify_func(this->m_results);
    printf("Return from verify %d\n", r);
    IOBuffer_dispose(&iobuf_ref);
    return r;
}
