#define _GNU_SOURCE

#include <c_http/http_parser/datasource.h>
#include <string.h>
#include <assert.h>
#include <c_http/common/alloc.h>

/**
 * @param this datasource_t*
 * @param blocks a pointer to an array of char* (an array of const cstring pointers)
 */
void datasource_init(datasource_t* this, char *blocks[])
{
    this->m_block_count = 0;
    this->m_blocks = blocks;
}

/**
 * Returns the next block of utf-8 null terminated data, NULL when done
 * @return char*, NULL when done
 */
char* datasource_next(datasource_t* this)
{
    char* block = this->m_blocks[this->m_block_count];
    this->m_block_count++;
    return block;
}

/**
 * @return bool true when no more data
 */
bool DataSource_finished(datasource_t* this)
{
    return (this->m_blocks[this->m_block_count] == NULL);
}

int datasource_read_some(datasource_t* this, void* buffer, int length)
{
    char* b0 = this->m_blocks[0];
    char* b1 = this->m_blocks[1];
    char* b2 = this->m_blocks[2];
    char* b3 = this->m_blocks[3];
    char* b4 = this->m_blocks[4];
    char* block = this->m_blocks[this->m_block_count];
    if (block == NULL) {
        return 0;
    } else if (strcmp(block, "error") == 0) {
        this->m_errno = -99;
        return -1;
    } else {
        this->m_block_count++;
        int block_len = (int)strlen(block);
        assert(block_len < length);
        memcpy((void*)buffer, block, block_len);
        return block_len;
    }
}
